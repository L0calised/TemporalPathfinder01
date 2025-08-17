#ifndef DATATYPES_H_INCLUDED
#define DATATYPES_H_INCLUDED

#include <iostream>
#include <vector>
#include <string>
#include <cstdio>

// --- Core Data Structures ---

struct Time {
    int h = 0, m = 0, s = 0;
    Time() = default;
    Time(const std::string& t_str) { sscanf(t_str.c_str(), "%d:%d:%d", &h, &m, &s); }
    int toSeconds() const { return h * 3600 + m * 60 + s; }
    static Time fromSeconds(int total_seconds) {
        Time t;
        t.h = total_seconds / 3600;
        t.m = (total_seconds % 3600) / 60;
        t.s = total_seconds % 60;
        return t;
    }
    bool operator>(const Time& other) const { return toSeconds() > other.toSeconds(); }
    bool operator<(const Time& other) const { return toSeconds() < other.toSeconds(); }
    bool operator>=(const Time& other) const { return toSeconds() >= other.toSeconds(); }
    bool operator<=(const Time& other) const { return toSeconds() <= other.toSeconds(); }
};

struct Stop { int id; std::string name; };
struct StopTime { std::string trip_id; Time arrival_time; Time departure_time; int stop_id; int stop_sequence; };
struct Transfer { int from_stop_id; int to_stop_id; int duration_seconds; };

// A multi-criteria journey "label"
struct Journey {
    Time arrival_time;
    int trips;
};

// --- Helper Functions ---

// Operator to print Time objects
std::ostream& operator<<(std::ostream& os, const Time& t);

#endif // DATATYPES_H_INCLUDED
