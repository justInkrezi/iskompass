// graph.cpp — Jeepney route graph for ISKOMPASS
// Adjacency list + Dijkstra's shortest path
#include "graph.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <climits>

// ────────────────────────────────────────────────
//  Stop destructor — frees adjacency list
// ────────────────────────────────────────────────
Stop::~Stop() {
    AdjNode* cur = neighbors;
    while (cur) {
        AdjNode* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
    neighbors = nullptr;
}

// ────────────────────────────────────────────────
//  Graph destructor — frees stop list
// ────────────────────────────────────────────────
Graph::~Graph() { clear(); }

void Graph::clear() {
    Stop* cur = head;
    while (cur) {
        Stop* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
    head = nullptr;
    numStops = 0;
}

// ────────────────────────────────────────────────
//  findStop — linear traversal of stop list
// ────────────────────────────────────────────────
Stop* Graph::findStop(const char* name) const {
    Stop* cur = head;
    while (cur) {
        if (strcmp(cur->name, name) == 0) return cur;
        cur = cur->next;
    }
    return nullptr;
}

// ────────────────────────────────────────────────
//  getOrCreate — find or insert stop at tail
// ────────────────────────────────────────────────
Stop* Graph::getOrCreate(const char* name) {
    Stop* existing = findStop(name);
    if (existing) return existing;

    Stop* s = new Stop();
    strncpy(s->name, name, 99);
    s->name[99] = '\0';

    // Append to tail of stop linked list
    if (!head) {
        head = s;
    } else {
        Stop* cur = head;
        while (cur->next) cur = cur->next;
        cur->next = s;
    }
    numStops++;
    return s;
}

// ────────────────────────────────────────────────
//  addEdge — append AdjNode to stop's adjacency list
//  Adds directed edge from→to (for undirected routes,
//  call twice with swapped arguments).
// ────────────────────────────────────────────────
void Graph::addEdge(const char* from, const char* to,
                    float base_time, const char* route) {
    Stop* src  = getOrCreate(from);
    Stop* dest = getOrCreate(to);

    AdjNode* node = new AdjNode();
    node->dest      = dest;
    node->base_time = base_time;
    strncpy(node->route, route, 9);
    node->route[9] = '\0';

    // Prepend to adjacency list (O(1))
    node->next      = src->neighbors;
    src->neighbors  = node;
}

// ────────────────────────────────────────────────
//  printGraph — debug dump
// ────────────────────────────────────────────────
void Graph::printGraph() const {
    Stop* cur = head;
    while (cur) {
        std::cout << "  [" << cur->name << "]\n";
        AdjNode* adj = cur->neighbors;
        while (adj) {
            std::cout << "      -> " << adj->dest->name
                      << "  (" << adj->base_time << " min, "
                      << adj->route << ")\n";
            adj = adj->next;
        }
        cur = cur->next;
    }
}

// ────────────────────────────────────────────────
//  CSV helpers
// ────────────────────────────────────────────────
static std::vector<std::string> splitCSV(const std::string& line) {
    std::vector<std::string> fields;
    std::string f;
    bool inQ = false;
    for (char c : line) {
        if (c == '"') { inQ = !inQ; }
        else if (c == ',' && !inQ) { fields.push_back(f); f.clear(); }
        else { f += c; }
    }
    fields.push_back(f);
    return fields;
}

// ────────────────────────────────────────────────
//  loadGraph — reads routes_data.csv
//  CSV columns: from_stop, to_stop, base_time, route, bidirectional
//  bidirectional: 1 = add reverse edge automatically
// ────────────────────────────────────────────────
Graph* loadGraph(const char* csvPath) {
    Graph* g = new Graph();
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        std::cerr << "[loadGraph] Cannot open: " << csvPath << "\n";
        return g;
    }

    std::string line;
    bool firstLine = true;
    while (std::getline(file, line)) {
        if (firstLine) { firstLine = false; continue; }
        if (line.empty()) continue;

        auto f = splitCSV(line);
        if (f.size() < 4) continue;

        const char* from  = f[0].c_str();
        const char* to    = f[1].c_str();
        float time        = std::stof(f[2]);
        const char* route = f[3].c_str();
        bool bidir        = (f.size() >= 5 && f[4] == "1");

        g->addEdge(from, to, time, route);
        if (bidir) g->addEdge(to, from, time, route);
    }
    file.close();
    std::cout << "[loadGraph] Loaded " << g->numStops << " stops.\n";
    return g;
}

// ────────────────────────────────────────────────
//  saveGraph — writes adjacency list back to CSV
// ────────────────────────────────────────────────
bool saveGraph(const Graph* g, const char* csvPath) {
    std::ofstream file(csvPath);
    if (!file.is_open()) {
        std::cerr << "[saveGraph] Cannot write: " << csvPath << "\n";
        return false;
    }
    file << "from_stop,to_stop,base_time,route,bidirectional\n";
    Stop* cur = g->head;
    while (cur) {
        AdjNode* adj = cur->neighbors;
        while (adj) {
            file << "\"" << cur->name       << "\","
                 << "\"" << adj->dest->name << "\","
                 << adj->base_time << ","
                 << adj->route     << ",0\n";
            adj = adj->next;
        }
        cur = cur->next;
    }
    file.close();
    return true;
}

// ────────────────────────────────────────────────
//  updateEdgeTime — admin: set new base_time for
//  all edges belonging to a given route code
// ────────────────────────────────────────────────
bool updateEdgeTime(Graph* g, const char* routeCode, float newBaseTime) {
    bool changed = false;
    Stop* cur = g->head;
    while (cur) {
        AdjNode* adj = cur->neighbors;
        while (adj) {
            if (strcmp(adj->route, routeCode) == 0) {
                adj->base_time = newBaseTime;
                changed = true;
            }
            adj = adj->next;
        }
        cur = cur->next;
    }
    return changed;
}

// ────────────────────────────────────────────────
//  calcFare — base ₱15, +₱2 per stop beyond 2
// ────────────────────────────────────────────────
int calcFare(int stopCount) {
    return std::max(15, 15 + (stopCount - 2) * 2);
}

// ────────────────────────────────────────────────
//  Dijkstra's Algorithm
//
//  Uses a simple linear-scan priority queue over
//  the stop linked list (appropriate for the small
//  campus/jeepney graph; no STL priority_queue
//  needed for prototype correctness).
//
//  Parameters:
//    g               — the loaded jeepney graph
//    startName       — name of origin stop
//    endName         — name of destination stop
//    applyTraffic    — multiply weights by trafficMultiplier
//    trafficMultiplier — default 1.3
//    filterRoute     — if non-null & non-empty, only traverse
//                       edges of that route code
// ────────────────────────────────────────────────
RouteResult dijkstra(const Graph* g,
                     const char* startName,
                     const char* endName,
                     bool        applyTraffic,
                     float       trafficMultiplier,
                     const char* filterRoute)
{
    RouteResult result;
    result.found             = false;
    result.totalBaseTime     = 0.0f;
    result.totalTimeTraffic  = 0.0f;
    result.estimatedFare     = 0;

    Stop* startStop = g->findStop(startName);
    Stop* endStop   = g->findStop(endName);

    if (!startStop) {
        std::cerr << "[dijkstra] Start stop not found: " << startName << "\n";
        return result;
    }
    if (!endStop) {
        std::cerr << "[dijkstra] End stop not found: " << endName << "\n";
        return result;
    }

    // ── Build index: stop pointer → index ──────────
    // Collect all stops into a vector for O(n) indexing
    std::vector<Stop*> allStops;
    {
        Stop* cur = g->head;
        while (cur) { allStops.push_back(cur); cur = cur->next; }
    }
    int n = (int)allStops.size();
    if (n == 0) return result;

    // ── dist[], prev[], prevRoute[] arrays ─────────
    const float INF = std::numeric_limits<float>::infinity();
    std::vector<float>  dist(n, INF);
    std::vector<int>    prev(n, -1);
    std::vector<std::string> prevRoute(n, "");
    std::vector<bool>   visited(n, false);

    // Helper: get index of a stop pointer
    auto indexOf = [&](Stop* s) -> int {
        for (int i = 0; i < n; i++)
            if (allStops[i] == s) return i;
        return -1;
    };

    int srcIdx = indexOf(startStop);
    int dstIdx = indexOf(endStop);
    if (srcIdx < 0 || dstIdx < 0) return result;

    dist[srcIdx] = 0.0f;

    // ── Main Dijkstra loop ─────────────────────────
    for (int iter = 0; iter < n; iter++) {
        // Pick unvisited stop with minimum distance (linear scan)
        int u = -1;
        for (int i = 0; i < n; i++) {
            if (!visited[i] && dist[i] < INF) {
                if (u == -1 || dist[i] < dist[u]) u = i;
            }
        }
        if (u == -1 || u == dstIdx) break;
        visited[u] = true;

        // Relax edges from allStops[u]
        AdjNode* adj = allStops[u]->neighbors;
        while (adj) {
            // Apply route filter if requested
            bool routeOk = (!filterRoute || filterRoute[0] == '\0' ||
                            strcmp(adj->route, filterRoute) == 0);
            if (routeOk) {
                int v = indexOf(adj->dest);
                if (v >= 0 && !visited[v]) {
                    float weight = applyTraffic
                                   ? adj->base_time * trafficMultiplier
                                   : adj->base_time;
                    float alt = dist[u] + weight;
                    if (alt < dist[v]) {
                        dist[v]      = alt;
                        prev[v]      = u;
                        prevRoute[v] = adj->route;
                    }
                }
            }
            adj = adj->next;
        }
    }

    // ── No path found ──────────────────────────────
    if (dist[dstIdx] == INF) return result;

    // ── Reconstruct path ───────────────────────────
    std::vector<RouteStep> steps;
    int cur = dstIdx;
    while (cur != -1) {
        RouteStep step;
        step.stopName   = allStops[cur]->name;
        step.route      = prevRoute[cur];
        step.timeToHere = dist[cur];
        steps.push_back(step);
        cur = prev[cur];
    }
    std::reverse(steps.begin(), steps.end());

    // ── Determine primary route (most frequent) ────
    std::unordered_map<std::string, int> routeFreq;
    for (auto& s : steps)
        if (!s.route.empty()) routeFreq[s.route]++;
    std::string primary;
    int maxFreq = 0;
    for (auto& kv : routeFreq)
        if (kv.second > maxFreq) { maxFreq = kv.second; primary = kv.first; }

    float baseTime = 0.0f;
    // Re-compute base time without traffic multiplier
    {
        // Walk path and sum base_time from graph edges
        for (size_t i = 0; i + 1 < steps.size(); i++) {
            Stop* from = g->findStop(steps[i].stopName.c_str());
            Stop* to   = g->findStop(steps[i+1].stopName.c_str());
            AdjNode* adj = from ? from->neighbors : nullptr;
            while (adj) {
                if (adj->dest == to) { baseTime += adj->base_time; break; }
                adj = adj->next;
            }
        }
    }

    result.found            = true;
    result.steps            = steps;
    result.totalBaseTime    = baseTime;
    result.totalTimeTraffic = applyTraffic
                              ? baseTime * trafficMultiplier
                              : baseTime;
    result.estimatedFare    = calcFare((int)steps.size());
    result.primaryRoute     = primary;

    return result;
}

// ────────────────────────────────────────────────
//  printRouteResult — console output
// ────────────────────────────────────────────────
void printRouteResult(const RouteResult& r) {
    if (!r.found) {
        std::cout << "  [No route found]\n";
        return;
    }
    std::cout << "  Route via " << r.primaryRoute << "\n";
    for (size_t i = 0; i < r.steps.size(); i++) {
        const auto& s = r.steps[i];
        std::cout << "  " << (i+1) << ". " << s.stopName;
        if (!s.route.empty()) std::cout << "  [" << s.route << "]";
        std::cout << "\n";
    }
    int lo = (int)r.totalTimeTraffic;
    int hi = (int)(r.totalTimeTraffic * 1.4f);
    std::cout << "  Estimated: " << lo << "–" << hi
              << " min (Traffic may vary)\n";
    std::cout << "  Fare: PHP " << r.estimatedFare << "\n";
}
