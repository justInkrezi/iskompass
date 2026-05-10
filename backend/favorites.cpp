// favorites.cpp — Per-user favorites (ISKOMPASS)
#include "favorites.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// ── FavList destructor ───────────────────────────
FavList::~FavList() { clear(); }

void FavList::clear() {
    FavNode* cur = head;
    while (cur) {
        FavNode* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
    head = nullptr;
    size = 0;
}

// ── Append — O(n) to avoid duplicates ───────────
void FavList::append(int buildingId) {
    if (contains(buildingId)) return;   // no duplicates
    FavNode* node = new FavNode(buildingId);
    if (!head) { head = node; size = 1; return; }
    FavNode* cur = head;
    while (cur->next) cur = cur->next;
    cur->next = node;
    size++;
}

// ── contains — linear scan ───────────────────────
bool FavList::contains(int buildingId) const {
    FavNode* cur = head;
    while (cur) {
        if (cur->buildingId == buildingId) return true;
        cur = cur->next;
    }
    return false;
}

// ── remove — unlink node with matching ID ────────
bool FavList::remove(int buildingId) {
    if (!head) return false;

    // Case: head is the target
    if (head->buildingId == buildingId) {
        FavNode* old = head;
        head = head->next;
        delete old;
        size--;
        return true;
    }

    // General case
    FavNode* prev = head;
    FavNode* cur  = head->next;
    while (cur) {
        if (cur->buildingId == buildingId) {
            prev->next = cur->next;
            delete cur;
            size--;
            return true;
        }
        prev = cur;
        cur  = cur->next;
    }
    return false;
}

// ── toVector — helper for iteration ─────────────
std::vector<int> FavList::toVector() const {
    std::vector<int> v;
    FavNode* cur = head;
    while (cur) { v.push_back(cur->buildingId); cur = cur->next; }
    return v;
}

// ────────────────────────────────────────────────
//  favorites.txt format (per line):
//    username,buildingId
//  e.g.:
//    iskostudent,3
//    iskostudent,12
//    anotheruser,7
// ────────────────────────────────────────────────

FavList* loadFavorites(const char* filePath, const char* username) {
    FavList* list = new FavList(username);
    std::ifstream file(filePath);
    if (!file.is_open()) {
        // File may not exist yet for a new user — that's fine
        return list;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string uname;
        int bid = 0;
        if (std::getline(ss, uname, ',') && (ss >> bid)) {
            if (uname == username) {
                list->append(bid);
            }
        }
    }
    file.close();
    std::cout << "[loadFavorites] " << username << " has "
              << list->size << " favorites.\n";
    return list;
}

// ── saveFavorites — rewrites the whole file ──────
// Reads all lines for OTHER users, keeps them,
// then appends this user's current favorites.
bool saveFavorites(const FavList* list, const char* filePath) {
    // Step 1: read all lines not belonging to this user
    std::vector<std::string> otherLines;
    {
        std::ifstream in(filePath);
        if (in.is_open()) {
            std::string line;
            while (std::getline(in, line)) {
                if (line.empty()) continue;
                std::istringstream ss(line);
                std::string uname;
                std::getline(ss, uname, ',');
                if (uname != list->username)
                    otherLines.push_back(line);
            }
            in.close();
        }
    }

    // Step 2: rewrite file
    std::ofstream out(filePath);
    if (!out.is_open()) {
        std::cerr << "[saveFavorites] Cannot write: " << filePath << "\n";
        return false;
    }
    for (auto& l : otherLines) out << l << "\n";
    FavNode* cur = list->head;
    while (cur) {
        out << list->username << "," << cur->buildingId << "\n";
        cur = cur->next;
    }
    out.close();
    return true;
}

// ── addFavorite — append to list + persist ───────
bool addFavorite(FavList* list, const char* filePath, int buildingId) {
    if (list->contains(buildingId)) {
        std::cout << "[addFavorite] Already saved: " << buildingId << "\n";
        return false;
    }
    list->append(buildingId);
    return saveFavorites(list, filePath);
}

// ── removeFavorite — remove + persist ────────────
bool removeFavorite(FavList* list, const char* filePath, int buildingId) {
    if (!list->remove(buildingId)) {
        std::cout << "[removeFavorite] ID not in list: " << buildingId << "\n";
        return false;
    }
    return saveFavorites(list, filePath);
}

// ── printFavorites — debug dump ──────────────────
void printFavorites(const FavList* list) {
    std::cout << "Favorites for [" << list->username << "]:\n";
    if (!list->head) { std::cout << "  (none)\n"; return; }
    FavNode* cur = list->head;
    int i = 1;
    while (cur) {
        std::cout << "  " << i++ << ". Building ID " << cur->buildingId << "\n";
        cur = cur->next;
    }
}
