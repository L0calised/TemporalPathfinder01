#ifndef RAPTOR_H_INCLUDED
#define RAPTOR_H_INCLUDED

#include <map>
#include <vector>
#include <string>
#include "DataTypes.h" // Include our new data types header

// Function declarations for the algorithm
void runMultiCriteriaRaptor(int start_stop_id, const Time& start_time,
                            const std::map<int, Stop>& stops,
                            const std::map<int, std::vector<Transfer>>& transfers_map,
                            const std::map<std::string, std::vector<StopTime>>& trips_map,
                            const std::map<int, std::vector<std::string>>& routes_serving_stop,
                            std::map<int, std::vector<Journey>>& final_profiles);

#endif // RAPTOR_H_INCLUDED
