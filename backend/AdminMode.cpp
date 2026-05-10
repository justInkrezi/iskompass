// AdminMode.cpp — Standalone Admin Utility for ISKOMPASS
// Run independently of the student-facing app.
// Compiles to a separate binary: g++ AdminMode.cpp buildings.cpp graph.cpp admin.cpp auth.cpp -o adminmode
//
// Usage:
//   ./adminmode                        — interactive menu
//   ./adminmode bldg <id> <name> <x> <y>
//   ./adminmode route <code> <time>
//   ./adminmode log [n]

#include "buildings.h"
#include "graph.h"
#include "admin.h"
#include "auth.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

static const char* PATH_CAMPUS = "../data/campus_data.csv";
static const char* PATH_ROUTES = "../data/routes_data.csv";
static const char* PATH_LOG    = "../data/admin_log.txt";
static const char* PATH_USERS  = "../data/users.txt";

// ── Authenticate admin before allowing operations ─
static User* adminAuth() {
    std::string uname, pass;
    std::cout << "Admin username: ";
    std::cin >> uname;
    std::cout << "Admin password: ";
    std::cin >> pass;
    std::cin.ignore();

    User* u = loginUser(PATH_USERS, uname.c_str(), pass.c_str());
    if (!u || u->role != UserRole::ADMIN) {
        delete u;
        std::cerr << "[AdminMode] Access denied.\n";
        return nullptr;
    }
    std::cout << "[AdminMode] Authenticated as " << u->full_name << ".\n\n";
    return u;
}

static void interactiveMenu(BuildingList* bldgs, Graph* g, User* admin) {
    std::cout <<
        "═══════════════════════════════════\n"
        "  ISKOMPASS Admin Mode\n"
        "═══════════════════════════════════\n"
        "  1. Update building (name/coords)\n"
        "  2. Update route base time\n"
        "  3. View all buildings\n"
        "  4. View graph (all stops)\n"
        "  5. View admin log\n"
        "  6. Add new user account\n"
        "  0. Exit\n"
        "═══════════════════════════════════\n";

    std::string line;
    while (true) {
        std::cout << "Choice: ";
        std::getline(std::cin, line);
        if (line.empty()) continue;

        int choice = std::stoi(line);
        switch (choice) {

        case 1: {
            int   id; float x, y; std::string name;
            std::cout << "Building ID : "; std::cin >> id;
            std::cout << "New name    : "; std::cin.ignore(); std::getline(std::cin, name);
            std::cout << "New X coord : "; std::cin >> x;
            std::cout << "New Y coord : "; std::cin >> y;
            std::cin.ignore();
            adminUpdateBuilding(bldgs, PATH_CAMPUS, id,
                                name.c_str(), x, y,
                                PATH_LOG, admin->username);
            break;
        }
        case 2: {
            std::string code; float t;
            std::cout << "Route code (e.g. 04L): "; std::cin >> code;
            std::cout << "New base time (min)   : "; std::cin >> t;
            std::cin.ignore();
            adminUpdateRoute(g, PATH_ROUTES, code.c_str(), t,
                             PATH_LOG, admin->username);
            break;
        }
        case 3:
            bldgs->printAll();
            break;
        case 4:
            std::cout << "Jeepney Graph:\n";
            g->printGraph();
            break;
        case 5: {
            int n = 15;
            std::cout << "How many entries? [15]: ";
            std::string ns; std::getline(std::cin, ns);
            if (!ns.empty()) n = std::stoi(ns);
            printAdminLog(PATH_LOG, n);
            break;
        }
        case 6: {
            std::string u, p, fname;
            std::cout << "New username : "; std::cin >> u;
            std::cout << "Password     : "; std::cin >> p;
            std::cout << "Full name    : "; std::cin.ignore(); std::getline(std::cin, fname);
            signupUser(PATH_USERS, u.c_str(), p.c_str(), fname.c_str());
            writeAdminLog(PATH_LOG, admin->username,
                          "user_created", u.c_str());
            break;
        }
        case 0:
            std::cout << "Exiting AdminMode.\n";
            return;
        default:
            std::cout << "Invalid choice.\n";
        }
        std::cout << "\n";
    }
}

int main(int argc, char* argv[]) {
    std::cout << "ISKOMPASS AdminMode — Loading data...\n";
    BuildingList* bldgs = loadBuildings(PATH_CAMPUS);
    Graph*        g     = loadGraph(PATH_ROUTES);

    // ── CLI mode ───────────────────────────────
    if (argc >= 2) {
        std::string subcmd = argv[1];
        User* admin = adminAuth();
        if (!admin) { delete bldgs; delete g; return 1; }

        if (subcmd == "bldg" && argc >= 6) {
            int   id = std::stoi(argv[2]);
            float x  = std::stof(argv[4]);
            float y  = std::stof(argv[5]);
            adminUpdateBuilding(bldgs, PATH_CAMPUS, id,
                                argv[3], x, y,
                                PATH_LOG, admin->username);
        } else if (subcmd == "route" && argc >= 4) {
            float t = std::stof(argv[3]);
            adminUpdateRoute(g, PATH_ROUTES, argv[2], t,
                             PATH_LOG, admin->username);
        } else if (subcmd == "log") {
            int n = (argc >= 3) ? std::stoi(argv[3]) : 15;
            printAdminLog(PATH_LOG, n);
        } else {
            std::cerr << "Usage:\n"
                      << "  adminmode bldg <id> <name> <x> <y>\n"
                      << "  adminmode route <code> <time>\n"
                      << "  adminmode log [n]\n";
        }
        delete admin;
        delete bldgs;
        delete g;
        return 0;
    }

    // ── Interactive mode ───────────────────────
    User* admin = adminAuth();
    if (!admin) { delete bldgs; delete g; return 1; }
    interactiveMenu(bldgs, g, admin);

    delete admin;
    delete bldgs;
    delete g;
    return 0;
}
