// admin.cpp — Admin utilities for ISKOMPASS
#include "admin.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstring>

// ────────────────────────────────────────────────
//  getTimestamp — local time as "YYYY-MM-DD HH:MM:SS"
// ────────────────────────────────────────────────
static std::string getTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm*    tm  = std::localtime(&now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
    return std::string(buf);
}

// ────────────────────────────────────────────────
//  writeAdminLog — append timestamped entry
//  Format: [YYYY-MM-DD HH:MM:SS] admin=X action=Y detail=Z
// ────────────────────────────────────────────────
bool writeAdminLog(const char* logPath,
                   const char* adminUsername,
                   const char* action,
                   const char* detail)
{
    std::ofstream file(logPath, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "[writeAdminLog] Cannot open: " << logPath << "\n";
        return false;
    }
    file << "[" << getTimestamp() << "]"
         << " admin=" << adminUsername
         << " action=" << action
         << " detail=" << detail << "\n";
    file.close();
    return true;
}

// ────────────────────────────────────────────────
//  printAdminLog — read last N lines and print
// ────────────────────────────────────────────────
void printAdminLog(const char* logPath, int lastN) {
    std::ifstream file(logPath);
    if (!file.is_open()) {
        std::cout << "  [Log file not found: " << logPath << "]\n";
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
        if (!line.empty()) lines.push_back(line);
    file.close();

    int start = std::max(0, (int)lines.size() - lastN);
    std::cout << "── Admin Log (last " << lastN << " entries) ──\n";
    for (int i = start; i < (int)lines.size(); i++)
        std::cout << "  " << lines[i] << "\n";
}

// ────────────────────────────────────────────────
//  getLastLogEntry — returns last non-empty line
// ────────────────────────────────────────────────
std::string getLastLogEntry(const char* logPath) {
    std::ifstream file(logPath);
    if (!file.is_open()) return "No log entries yet.";

    std::string last, line;
    while (std::getline(file, line))
        if (!line.empty()) last = line;
    file.close();
    return last.empty() ? "No log entries yet." : last;
}

// ────────────────────────────────────────────────
//  adminUpdateBuilding — update name/coords in
//  the in-memory list AND persist to campus_data.csv
// ────────────────────────────────────────────────
bool adminUpdateBuilding(BuildingList* list,
                         const char*   csvPath,
                         int           buildingId,
                         const char*   newName,
                         float         newX,
                         float         newY,
                         const char*   logPath,
                         const char*   adminUsername)
{
    if (!list) { std::cerr << "[adminUpdateBuilding] Null list.\n"; return false; }

    Building* b = list->findById(buildingId);
    if (!b) {
        std::cerr << "[adminUpdateBuilding] Building ID not found: "
                  << buildingId << "\n";
        return false;
    }

    // Apply changes
    if (newName && strlen(newName) > 0)
        strncpy(b->name, newName, 99);
    if (newX >= 0.0f) b->x = newX;
    if (newY >= 0.0f) b->y = newY;

    // Persist
    bool saved = saveBuildings(list, csvPath);
    if (!saved) {
        std::cerr << "[adminUpdateBuilding] Failed to save CSV.\n";
        return false;
    }

    // Log
    std::ostringstream detail;
    detail << "id=" << buildingId
           << " name=\"" << b->name << "\""
           << " x=" << newX << " y=" << newY;
    writeAdminLog(logPath, adminUsername, "building_update", detail.str().c_str());

    std::cout << "[adminUpdateBuilding] Updated building #"
              << buildingId << " → " << b->name << "\n";
    return true;
}

// ────────────────────────────────────────────────
//  adminUpdateRoute — change base_time for all
//  edges of a given route code, then persist
// ────────────────────────────────────────────────
bool adminUpdateRoute(Graph*      g,
                      const char* csvPath,
                      const char* routeCode,
                      float       newBaseTime,
                      const char* logPath,
                      const char* adminUsername)
{
    if (!g) { std::cerr << "[adminUpdateRoute] Null graph.\n"; return false; }
    if (newBaseTime <= 0) {
        std::cerr << "[adminUpdateRoute] Invalid time: " << newBaseTime << "\n";
        return false;
    }

    bool changed = updateEdgeTime(g, routeCode, newBaseTime);
    if (!changed) {
        std::cerr << "[adminUpdateRoute] Route code not found: "
                  << routeCode << "\n";
        return false;
    }

    bool saved = saveGraph(g, csvPath);
    if (!saved) {
        std::cerr << "[adminUpdateRoute] Failed to save routes CSV.\n";
        return false;
    }

    std::ostringstream detail;
    detail << "route=" << routeCode << " base_time=" << newBaseTime << "min";
    writeAdminLog(logPath, adminUsername, "route_update", detail.str().c_str());

    std::cout << "[adminUpdateRoute] Route " << routeCode
              << " updated to " << newBaseTime << " min.\n";
    return true;
}
