#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include "Raptor.h" // Include the header for this file

// The multi-criteria merge function
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

// The Multi-Criteria Profile RAPTOR Algorithm
void runMultiCriteriaRaptor(int start_stop_id, const Time& start_time,
                            const std::map<int, Stop>& stops,
                            const std::map<int, std::vector<Transfer>>& transfers_map,
                            const std::map<std::string, std::vector<StopTime>>& trips_map,
                            const std::map<int, std::vector<std::string>>& routes_serving_stop,
                            std::map<int, std::vector<Journey>>& final_profiles) {

    const int MAX_TRIPS = 5;
    std::vector<std::map<int, std::vector<Journey>>> profiles_by_round(MAX_TRIPS + 1);

    merge(profiles_by_round[0][start_stop_id], {start_time, 0});
    if (transfers_map.count(start_stop_id)) {
        for (const auto& transfer : transfers_map.at(start_stop_id)) {
            merge(profiles_by_round[0][transfer.to_stop_id],
                {Time::fromSeconds(start_time.toSeconds() + transfer.duration_seconds), 0});
        }
    }

    for (int k = 1; k <= MAX_TRIPS; ++k) {
        std::set<std::string> routes_to_scan;
        for (const auto& pair : profiles_by_round[k - 1]) {
            if (routes_serving_stop.count(pair.first)) {
                for (const auto& route_id : routes_serving_stop.at(pair.first)) routes_to_scan.insert(route_id);
            }
        }
        for (const auto& trip_id : routes_to_scan) {
            Journey earliest_journey_on_board = {Time("23:59:59"), MAX_TRIPS + 1};
            const auto& schedule = trips_map.at(trip_id);
            for (const auto& stop_time : schedule) {
                if (profiles_by_round[k - 1].count(stop_time.stop_id)) {
                    for (const auto& prev_journey : profiles_by_round[k - 1].at(stop_time.stop_id)) {
                        if (prev_journey.arrival_time <= stop_time.departure_time) {
                             if (prev_journey.arrival_time < earliest_journey_on_board.arrival_time) {
                                 earliest_journey_on_board = prev_journey;
                             }
                        }
                    }
                }
                if (!(earliest_journey_on_board.arrival_time > stop_time.arrival_time)) {
                    Journey new_journey = {stop_time.arrival_time, k};
                    merge(profiles_by_round[k][stop_time.stop_id], new_journey);
                }
            }
        }
        for (const auto& pair : profiles_by_round[k]) {
            int from_stop = pair.first;
            for (const auto& journey : pair.second) {
                if (transfers_map.count(from_stop)) {
                    for (const auto& transfer : transfers_map.at(from_stop)) {
                        Journey transfer_journey = {Time::fromSeconds(journey.arrival_time.toSeconds() + transfer.duration_seconds), journey.trips};
                        merge(profiles_by_round[k][transfer.to_stop_id], transfer_journey);
                    }
                }
            }
        }
    }

    for (const auto& stop_pair : stops) {
        for (int k = 0; k <= MAX_TRIPS; ++k) {
            if (profiles_by_round[k].count(stop_pair.first)) {
                for (const auto& journey : profiles_by_round[k].at(stop_pair.first)) {
                    merge(final_profiles[stop_pair.first], journey);
                }
            }
        }
    }
}
