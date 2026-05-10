#pragma once
// favorites.h — Per-user favorites list (ISKOMPASS)
// Persistence: favorites.txt (one BuildingID per line, prefixed by username)
// In-memory: singly linked list of FavNode

#include <string>
#include <vector>

// ── In-memory favorites node ─────────────────────
struct FavNode {
    int      buildingId;
    FavNode* next;

    explicit FavNode(int id) : buildingId(id), next(nullptr) {}
};

// ── Favorites linked list (one per logged-in user) ─
struct FavList {
    FavNode* head;
    int      size;
    std::string username;   // owner of this list

    explicit FavList(const std::string& user)
        : head(nullptr), size(0), username(user) {}
    ~FavList();

    void append(int buildingId);
    bool contains(int buildingId) const;
    bool remove(int buildingId);
    std::vector<int> toVector() const;
    void clear();
};

// ── Public API ───────────────────────────────────
FavList* loadFavorites(const char* filePath, const char* username);
bool     saveFavorites(const FavList* list, const char* filePath);
bool     addFavorite(FavList* list, const char* filePath, int buildingId);
bool     removeFavorite(FavList* list, const char* filePath, int buildingId);
void     printFavorites(const FavList* list);
