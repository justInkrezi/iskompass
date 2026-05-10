#pragma once
// graph.h — Adjacency list graph for jeepney stops (ISKOMPASS)
// Data structure: singly linked list of Stop nodes,
//                 each with an adjacency list of AdjNode neighbors

#include <string>
#include <vector>
#include <unordered_map>
#include <limits>

// ── Forward declaration ──────────────────────────
struct Stop;

// ── Adjacency list node ──────────────────────────
struct AdjNode {
    Stop*    dest;
    float    base_time;   // minutes (unweighted by traffic)
    char     route[10];   // e.g. "04L", "12L"
    AdjNode* next;

    AdjNode() : dest(nullptr), base_time(0.0f), next(nullptr) {
        route[0] = '\0';
    }
};

// ── Stop node ───────────────────────────────────
struct Stop {
    char     name[100];
    AdjNode* neighbors;   // head of adjacency list
    Stop*    next;        // next stop in the stop list

    Stop() : neighbors(nullptr), next(nullptr) {
        name[0] = '\0';
    }
    ~Stop();
};

// ── Graph (collection of stops) ─────────────────
struct Graph {
    Stop* head;   // head of stop linked list
    int   numStops;

    Graph() : head(nullptr), numStops(0) {}
    ~Graph();

    Stop*  findStop(const char* name) const;
    Stop*  getOrCreate(const char* name);
    void   addEdge(const char* from, const char* to,
                   float base_time, const char* route);
    void   printGraph() const;
    void   clear();
};

// ── Route result structures ──────────────────────
struct RouteStep {
    std::string stopName;
    std::string route;      // jeepney line used to arrive here
    float       timeToHere; // cumulative time from origin
};

struct RouteResult {
    bool                    found;
    std::vector<RouteStep>  steps;
    float                   totalBaseTime;   // minutes, no traffic
    float                   totalTimeTraffic;// minutes, with multiplier
    int                     estimatedFare;   // PHP
    std::string             primaryRoute;
};

// ── Public API ───────────────────────────────────
Graph*      loadGraph(const char* csvPath);
bool        saveGraph(const Graph* g, const char* csvPath);
bool        updateEdgeTime(Graph* g, const char* routeCode, float newBaseTime);
RouteResult dijkstra(const Graph* g,
                     const char* startName,
                     const char* endName,
                     bool        applyTraffic,
                     float       trafficMultiplier = 1.3f,
                     const char* filterRoute = nullptr);
void        printRouteResult(const RouteResult& r);
int         calcFare(int stopCount);
