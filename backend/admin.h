#pragma once
// admin.h — Admin data editor, route manager, system log (ISKOMPASS)

#include "buildings.h"
#include "graph.h"
#include <string>

// ── Public API ───────────────────────────────────
bool adminUpdateBuilding(BuildingList* list,
                         const char*   csvPath,
                         int           buildingId,
                         const char*   newName,
                         float         newX,
                         float         newY,
                         const char*   logPath,
                         const char*   adminUsername);

bool adminUpdateRoute(Graph*      g,
                      const char* csvPath,
                      const char* routeCode,
                      float       newBaseTime,
                      const char* logPath,
                      const char* adminUsername);

// Writes a timestamped entry to admin_log.txt
bool  writeAdminLog(const char* logPath,
                    const char* adminUsername,
                    const char* action,
                    const char* detail);

// Reads and prints the last N lines of admin_log.txt
void  printAdminLog(const char* logPath, int lastN = 10);

// Returns the last log entry as a string (for Settings display)
std::string getLastLogEntry(const char* logPath);
