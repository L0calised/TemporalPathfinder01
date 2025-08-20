#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include "Raptor.h"
#include "DataTypes.h"

const double WALKING_SPEED_MPS = 1.4;
const double MAX_WALK_DISTANCE_METERS = 1500;

void merge(std::vector<Journey>& profile, const Journey& new_journey) {
    for (const auto& existing : profile) {
        if (existing.arrival_time <= new_journey.arrival_time && existing.trips <= new_journey.trips) {
            return;
        }
    }
    profile.erase(std::remove_if(profile.begin(), profile.end(),
        [&](const Journey& existing) {
            return new_journey.arrival_time <= existing.arrival_time && new_journey.trips <= existing.trips;
        }),
    profile.end());
    profile.push_back(new_journey);
}

void runMultiCriteriaRaptor(int start_stop_id, int end_stop_id, const Time& start_time,
                            const std::map<int, Stop>& stops,
                            const std::map<int, std::vector<Transfer>>& transfers_map,
                            const std::map<std::string, std::vector<StopTime>>& trips_map,
                            const std::map<int, std::vector<std::string>>& routes_serving_stop,
                            std::map<int, std::vector<Journey>>& final_profiles,
                            std::map<int, std::map<int, Journey>>& predecessors) {

    const int MAX_TRIPS = 5;
    std::vector<std::map<int, std::vector<Journey>>> profiles_by_round(MAX_TRIPS + 1);

    // Round 0: Initialize
    merge(profiles_by_round[0][start_stop_id], {start_time, 0, start_time, -1, "Start"});
    const Stop& start_stop_details = stops.at(start_stop_id);
    for (const auto& stop_pair : stops) {
        double distance = haversine(start_stop_details.lat, start_stop_details.lon, stop_pair.second.lat, stop_pair.second.lon);
        if (distance <= MAX_WALK_DISTANCE_METERS && stop_pair.first != start_stop_id) {
            int walk_duration_seconds = static_cast<int>(distance / WALKING_SPEED_MPS);
            Journey j = {Time::fromSeconds(start_time.toSeconds() + walk_duration_seconds), 0, start_time, start_stop_id, "Walk"};
            merge(profiles_by_round[0][stop_pair.first], j);
        }
    }
    if (transfers_map.count(start_stop_id)) {
        for (const auto& transfer : transfers_map.at(start_stop_id)) {
            Journey j = {Time::fromSeconds(start_time.toSeconds() + transfer.duration_seconds), 0, start_time, start_stop_id, "Walk"};
            merge(profiles_by_round[0][transfer.to_stop_id], j);
        }
    }

    // --- NEW, HIGH-PERFORMANCE RAPTOR ROUNDS ---
    for (int k = 1; k <= MAX_TRIPS; ++k) {
        std::map<int, std::vector<Journey>> newly_reached_stops;
        std::set<std::string> marked_trips;

        // Collect all trips that serve stops reached in the previous round
        for (const auto& pair : profiles_by_round[k - 1]) {
            if (routes_serving_stop.count(pair.first)) {
                for (const auto& trip_id : routes_serving_stop.at(pair.first)) {
                    marked_trips.insert(trip_id);
                }
            }
        }

        // Process each marked trip only once
        for (const auto& trip_id : marked_trips) {
            Journey current_trip_journey;
            current_trip_journey.from_stop_id = -1; // Sentinel: not on board yet
            int boarding_stop_seq_index = -1;

            const auto& schedule = trips_map.at(trip_id);
            for (size_t i = 0; i < schedule.size(); ++i) {
                const auto& stop_time = schedule[i];

                if (profiles_by_round[k - 1].count(stop_time.stop_id)) {
                    for (const auto& prev_journey : profiles_by_round[k - 1].at(stop_time.stop_id)) {
                        if (prev_journey.arrival_time <= stop_time.departure_time) {
                            if (current_trip_journey.from_stop_id == -1 || prev_journey.arrival_time < current_trip_journey.arrival_time) {
                                current_trip_journey = prev_journey;
                                boarding_stop_seq_index = i;
                            }
                        }
                    }
                }

                if (current_trip_journey.from_stop_id != -1) {
                    int predecessor_id = (i > static_cast<size_t>(boarding_stop_seq_index)) ? schedule[i - 1].stop_id : current_trip_journey.from_stop_id;
                    Journey new_journey = {stop_time.arrival_time, k, current_trip_journey.departure_time, predecessor_id, "Trip " + trip_id};
                    merge(newly_reached_stops[stop_time.stop_id], new_journey);
                }
            }
        }

        // Add transfers from newly reached stops
        for (const auto& pair : newly_reached_stops) {
            for (const auto& journey : pair.second) {
                merge(profiles_by_round[k][pair.first], journey); // Add trip results to main profile
                if (transfers_map.count(pair.first)) {
                    for (const auto& transfer : transfers_map.at(pair.first)) {
                        Journey transfer_journey = {Time::fromSeconds(journey.arrival_time.toSeconds() + transfer.duration_seconds), journey.trips, journey.departure_time, pair.first, "Walk"};
                        merge(profiles_by_round[k][transfer.to_stop_id], transfer_journey);
                    }
                }
            }
        }
    }

    // --- EFFICIENT FINALIZATION LOGIC ---
    std::map<int, std::vector<Journey>> temp_final_profiles;
    for (int k = 0; k <= MAX_TRIPS; ++k) {
        for (const auto& profile_pair : profiles_by_round[k]) {
            int stop_id = profile_pair.first;
            for (const auto& journey : profile_pair.second) {
                merge(temp_final_profiles[stop_id], journey);
            }
        }
    }

    for (const auto& profile_pair : temp_final_profiles) {
        int reached_stop_id = profile_pair.first;
        const Stop& reached_stop_details = stops.at(reached_stop_id);
        double distance = haversine(reached_stop_details.lat, reached_stop_details.lon, stops.at(end_stop_id).lat, stops.at(end_stop_id).lon);
        if (distance <= MAX_WALK_DISTANCE_METERS) {
            int walk_duration_seconds = static_cast<int>(distance / WALKING_SPEED_MPS);
            for (const auto& journey : profile_pair.second) {
                 Journey final_walk = {Time::fromSeconds(journey.arrival_time.toSeconds() + walk_duration_seconds), journey.trips, journey.departure_time, reached_stop_id, "Walk"};
                merge(final_profiles[end_stop_id], final_walk);
            }
        }
    }

    for (const auto& pair : temp_final_profiles) {
        for (const auto& journey : pair.second) {
            merge(final_profiles[pair.first], journey);
        }
    }

    for(const auto& profile_pair : final_profiles) {
        for(const auto& journey : profile_pair.second) {
            predecessors[profile_pair.first][journey.trips] = journey;
        }
    }
}
