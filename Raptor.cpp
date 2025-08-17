#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include "Raptor.h"

// The multi-criteria merge function
void merge(std::vector<Journey>& profile, const Journey& new_journey) {
    for (const auto& existing : profile) {
        if (existing.arrival_time <= new_journey.arrival_time && existing.trips <= new_journey.trips) {
            return; // New journey is dominated
        }
    }
    profile.erase(std::remove_if(profile.begin(), profile.end(),
        [&](const Journey& existing) {
            return new_journey.arrival_time <= existing.arrival_time && new_journey.trips <= existing.trips;
        }),
    profile.end());
    profile.push_back(new_journey);
}

// The Multi-Criteria Profile RAPTOR Algorithm implementation
void runMultiCriteriaRaptor(int start_stop_id, const Time& start_time,
                            const std::map<int, Stop>& stops,
                            const std::map<int, std::vector<Transfer>>& transfers_map,
                            const std::map<std::string, std::vector<StopTime>>& trips_map,
                            const std::map<int, std::vector<std::string>>& routes_serving_stop,
                            std::map<int, std::vector<Journey>>& final_profiles,
                            std::map<int, std::map<int, Journey>>& predecessors) {

    const int MAX_TRIPS = 5;
    std::vector<std::map<int, std::vector<Journey>>> profiles_by_round(MAX_TRIPS + 1);

    // Round 0: Initialize with start time and direct walks
    merge(profiles_by_round[0][start_stop_id], {start_time, 0, start_time, -1, "Start"});
    if (transfers_map.count(start_stop_id)) {
        for (const auto& transfer : transfers_map.at(start_stop_id)) {
            Journey j = {Time::fromSeconds(start_time.toSeconds() + transfer.duration_seconds), 0, start_time, start_stop_id, "Walk"};
            merge(profiles_by_round[0][transfer.to_stop_id], j);
        }
    }

    // RAPTOR Rounds
    for (int k = 1; k <= MAX_TRIPS; ++k) {
        std::set<std::string> routes_to_scan;
        for (const auto& pair : profiles_by_round[k - 1]) {
            if (routes_serving_stop.count(pair.first)) {
                for (const auto& route_id : routes_serving_stop.at(pair.first)) routes_to_scan.insert(route_id);
            }
        }
        for (const auto& trip_id : routes_to_scan) {
            Journey earliest_journey_on_board = {Time("23:59:59"), MAX_TRIPS + 1, Time("23:59:59")};
            int boarding_stop = -1;
            const auto& schedule = trips_map.at(trip_id);
            for (const auto& stop_time : schedule) {
                if (profiles_by_round[k - 1].count(stop_time.stop_id)) {
                    for (const auto& prev_journey : profiles_by_round[k - 1].at(stop_time.stop_id)) {
                        if (prev_journey.arrival_time <= stop_time.departure_time) {
                             if (prev_journey.arrival_time < earliest_journey_on_board.arrival_time) {
                                 earliest_journey_on_board = prev_journey;
                                 boarding_stop = stop_time.stop_id;
                             }
                        }
                    }
                }
                if (!(earliest_journey_on_board.arrival_time > stop_time.arrival_time)) {
                    Journey new_journey = {stop_time.arrival_time, k, earliest_journey_on_board.departure_time, boarding_stop, "Trip " + trip_id};
                    merge(profiles_by_round[k][stop_time.stop_id], new_journey);
                }
            }
        }
        for (const auto& pair : profiles_by_round[k]) {
            for (const auto& journey : pair.second) {
                if (transfers_map.count(pair.first)) {
                    for (const auto& transfer : transfers_map.at(pair.first)) {
                        Journey transfer_journey = {Time::fromSeconds(journey.arrival_time.toSeconds() + transfer.duration_seconds), journey.trips, journey.departure_time, pair.first, "Walk"};
                        merge(profiles_by_round[k][transfer.to_stop_id], transfer_journey);
                    }
                }
            }
        }
    }

    // Combine final results
    for (const auto& stop_pair : stops) {
        for (int k = 0; k <= MAX_TRIPS; ++k) {
            if (profiles_by_round[k].count(stop_pair.first)) {
                for (const auto& journey : profiles_by_round[k].at(stop_pair.first)) {
                    merge(final_profiles[stop_pair.first], journey);
                    predecessors[stop_pair.first][journey.trips] = journey;
                }
            }
        }
    }
}
