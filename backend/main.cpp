// main.cpp — ISKOMPASS Backend Entry Point
// Command-line interface that routes to all subsystems.
// In a full deployment this would be a lightweight HTTP server (e.g. cpp-httplib).
// For the prototype it is a REPL / command dispatcher.
//
// Commands:
//   login    <username> <password>
//   signup   <username> <password> <fullname>
//   buildings                        — list all
//   building  <id>                   — show detail
//   search   <query>                 — search buildings
//   route    <start> <end> [traffic] — find jeepney route
//   favorites                        — list saved buildings
//   fav-add  <buildingId>
//   fav-rm   <buildingId>
//   admin-bldg <id> <name> <x> <y>  — update building
//   admin-route <code> <time>        — update route time
//   log                              — show admin log
//   quit / exit

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>

#include "buildings.h"
#include "graph.h"
#include "favorites.h"
#include "auth.h"
#include "admin.h"

// ── Data file paths ───────────────────────────────
static const char* PATH_CAMPUS   = "../data/campus_data.csv";
static const char* PATH_ROUTES   = "../data/routes_data.csv";
static const char* PATH_FAVS     = "../data/favorites.txt";
static const char* PATH_USERS    = "../data/users.txt";
static const char* PATH_LOG      = "../data/admin_log.txt";

// ── Session state ─────────────────────────────────
static User*         gCurrentUser  = nullptr;
static BuildingList* gBuildings    = nullptr;
static Graph*        gGraph        = nullptr;
static FavList*      gFavorites    = nullptr;

// ── Helper: split a line into tokens ─────────────
static std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream ss(line);
    std::string tok;
    bool inQuote = false;
    std::string cur;
    for (char c : line) {
        if (c == '"') { inQuote = !inQuote; }
        else if (c == ' ' && !inQuote) {
            if (!cur.empty()) { tokens.push_back(cur); cur.clear(); }
        } else { cur += c; }
    }
    if (!cur.empty()) tokens.push_back(cur);
    return tokens;
}

// ── Print session banner ──────────────────────────
static void printBanner() {
    std::cout <<
        "╔═══════════════════════════════════════╗\n"
        "║       ISKOMPASS Backend v1.0          ║\n"
        "║   UP Cebu Campus Guide — Prototype    ║\n"
        "╚═══════════════════════════════════════╝\n"
        "Type 'help' for available commands.\n\n";
}

// ── Print help ────────────────────────────────────
static void printHelp() {
    std::cout <<
        "Commands:\n"
        "  login    <user> <pass>          — Authenticate\n"
        "  signup   <user> <pass> <name>   — Register new student\n"
        "  logout                          — End session\n"
        "  buildings                       — List all buildings\n"
        "  building  <id>                  — Show building detail\n"
        "  search    <query>               — Search by name/tag\n"
        "  route     <start> <end>         — Find jeepney route\n"
        "  route     <start> <end> traffic — Include traffic delay\n"
        "  favorites                       — List your favorites\n"
        "  fav-add   <buildingId>          — Save a favorite\n"
        "  fav-rm    <buildingId>          — Remove a favorite\n"
        "  admin-bldg <id> <name> <x> <y> — [ADMIN] Update building\n"
        "  admin-route <code> <time>       — [ADMIN] Update route time\n"
        "  log                             — Show admin log\n"
        "  quit / exit\n";
}

// ── Require login guard ───────────────────────────
static bool requireLogin() {
    if (!gCurrentUser) {
        std::cout << "  [!] Please log in first.\n";
        return false;
    }
    return true;
}

// ── Require admin guard ───────────────────────────
static bool requireAdmin() {
    if (!requireLogin()) return false;
    if (gCurrentUser->role != UserRole::ADMIN) {
        std::cout << "  [!] Admin access required.\n";
        return false;
    }
    return true;
}

// ════════════════════════════════════════════════
//  Command Handlers
// ════════════════════════════════════════════════

static void cmdLogin(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: login <username> <password>\n"; return;
    }
    delete gCurrentUser;
    gCurrentUser = loginUser(PATH_USERS, args[1].c_str(), args[2].c_str());
    if (gCurrentUser) {
        std::cout << "  Welcome, " << gCurrentUser->full_name << "!\n";
        // Load this user's favorites
        delete gFavorites;
        gFavorites = loadFavorites(PATH_FAVS, gCurrentUser->username);
    }
}

static void cmdSignup(const std::vector<std::string>& args) {
    if (args.size() < 4) {
        std::cout << "Usage: signup <username> <password> <fullname>\n"; return;
    }
    // Join remaining args as full name
    std::string fname;
    for (size_t i = 3; i < args.size(); i++) {
        if (i > 3) fname += ' ';
        fname += args[i];
    }
    signupUser(PATH_USERS, args[1].c_str(), args[2].c_str(), fname.c_str());
}

static void cmdLogout() {
    if (gCurrentUser) {
        std::cout << "  Goodbye, " << gCurrentUser->full_name << "!\n";
        delete gCurrentUser; gCurrentUser = nullptr;
        delete gFavorites;   gFavorites   = nullptr;
    } else {
        std::cout << "  Not logged in.\n";
    }
}

static void cmdBuildings() {
    if (!gBuildings || !gBuildings->head) {
        std::cout << "  No building data loaded.\n"; return;
    }
    gBuildings->printAll();
}

static void cmdBuilding(const std::vector<std::string>& args) {
    if (args.size() < 2) { std::cout << "Usage: building <id>\n"; return; }
    int id = std::stoi(args[1]);
    Building* b = gBuildings->findById(id);
    if (!b) { std::cout << "  Building #" << id << " not found.\n"; return; }
    printBuildingDetail(b);
}

static void cmdSearch(const std::vector<std::string>& args) {
    if (args.size() < 2) { std::cout << "Usage: search <query>\n"; return; }
    std::string query;
    for (size_t i = 1; i < args.size(); i++) {
        if (i > 1) query += ' ';
        query += args[i];
    }
    auto results = searchBuildings(gBuildings, query.c_str());
    std::cout << "  Found " << results.size() << " result(s) for \""
              << query << "\":\n";
    for (Building* b : results) printBuildingDetail(b);
}

static void cmdRoute(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Usage: route <start> <end> [traffic]\n"; return;
    }
    // start and end may be multi-word; convention: use underscore in CLI
    // e.g.  route Colon_Street UP_Cebu_Entrance_Gate traffic
    std::string start = args[1], end = args[2];
    // Replace underscores with spaces
    for (char& c : start) if (c == '_') c = ' ';
    for (char& c : end)   if (c == '_') c = ' ';

    bool traffic = (args.size() >= 4 && args[3] == "traffic");

    std::cout << "  Finding route: [" << start << "] → [" << end << "]"
              << (traffic ? " (with traffic)" : "") << "\n";

    RouteResult r = dijkstra(gGraph, start.c_str(), end.c_str(), traffic);
    printRouteResult(r);
}

static void cmdFavorites() {
    if (!requireLogin()) return;
    printFavorites(gFavorites);
}

static void cmdFavAdd(const std::vector<std::string>& args) {
    if (!requireLogin()) return;
    if (args.size() < 2) { std::cout << "Usage: fav-add <buildingId>\n"; return; }
    int id = std::stoi(args[1]);
    // Verify building exists
    if (!gBuildings->findById(id)) {
        std::cout << "  Building #" << id << " not found.\n"; return;
    }
    addFavorite(gFavorites, PATH_FAVS, id);
    std::cout << "  Saved building #" << id << " to favorites.\n";
}

static void cmdFavRm(const std::vector<std::string>& args) {
    if (!requireLogin()) return;
    if (args.size() < 2) { std::cout << "Usage: fav-rm <buildingId>\n"; return; }
    int id = std::stoi(args[1]);
    removeFavorite(gFavorites, PATH_FAVS, id);
    std::cout << "  Removed building #" << id << " from favorites.\n";
}

static void cmdAdminBldg(const std::vector<std::string>& args) {
    if (!requireAdmin()) return;
    if (args.size() < 5) {
        std::cout << "Usage: admin-bldg <id> <name> <x> <y>\n"; return;
    }
    int   id   = std::stoi(args[1]);
    float x    = std::stof(args[3]);
    float y    = std::stof(args[4]);
    adminUpdateBuilding(gBuildings, PATH_CAMPUS, id,
                        args[2].c_str(), x, y,
                        PATH_LOG, gCurrentUser->username);
}

static void cmdAdminRoute(const std::vector<std::string>& args) {
    if (!requireAdmin()) return;
    if (args.size() < 3) {
        std::cout << "Usage: admin-route <code> <base_time_min>\n"; return;
    }
    float t = std::stof(args[2]);
    adminUpdateRoute(gGraph, PATH_ROUTES, args[1].c_str(), t,
                     PATH_LOG, gCurrentUser->username);
}

static void cmdLog() {
    printAdminLog(PATH_LOG, 15);
}

// ════════════════════════════════════════════════
//  main — REPL loop
// ════════════════════════════════════════════════
int main() {
    printBanner();

    // ── Load data ──────────────────────────────
    std::cout << "Loading campus data...\n";
    gBuildings = loadBuildings(PATH_CAMPUS);
    gGraph     = loadGraph(PATH_ROUTES);
    std::cout << "Ready.\n\n";

    std::string line;
    while (true) {
        if (gCurrentUser)
            std::cout << "[" << gCurrentUser->username << "] > ";
        else
            std::cout << "[guest] > ";

        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        auto args = tokenize(line);
        if (args.empty()) continue;

        const std::string& cmd = args[0];

        if (cmd == "quit" || cmd == "exit") {
            std::cout << "Goodbye!\n";
            break;
        } else if (cmd == "help") {
            printHelp();
        } else if (cmd == "login") {
            cmdLogin(args);
        } else if (cmd == "signup") {
            cmdSignup(args);
        } else if (cmd == "logout") {
            cmdLogout();
        } else if (cmd == "buildings") {
            cmdBuildings();
        } else if (cmd == "building") {
            cmdBuilding(args);
        } else if (cmd == "search") {
            cmdSearch(args);
        } else if (cmd == "route") {
            cmdRoute(args);
        } else if (cmd == "favorites") {
            cmdFavorites();
        } else if (cmd == "fav-add") {
            cmdFavAdd(args);
        } else if (cmd == "fav-rm") {
            cmdFavRm(args);
        } else if (cmd == "admin-bldg") {
            cmdAdminBldg(args);
        } else if (cmd == "admin-route") {
            cmdAdminRoute(args);
        } else if (cmd == "log") {
            cmdLog();
        } else {
            std::cout << "  Unknown command: " << cmd
                      << ". Type 'help' for options.\n";
        }
    }

    // ── Cleanup ────────────────────────────────
    delete gCurrentUser;
    delete gBuildings;
    delete gGraph;
    delete gFavorites;
    return 0;
}
