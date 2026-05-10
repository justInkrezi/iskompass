#pragma once
// buildings.h — Building linked list for ISKOMPASS
// Data structure: singly linked list of Building nodes

#include <string>
#include <vector>
#include <cstring>

#define MAX_TAGS 5

struct Building {
    int   id;
    char  name[100];
    char  photo_path[200];
    char  services[300];
    char  hours[100];
    char  in_charge[100];
    char  tags[MAX_TAGS][50];
    float x, y;
    char  qr_url[300];
    Building* next;

    Building() : id(0), x(0.0f), y(0.0f), next(nullptr) {
        name[0] = photo_path[0] = services[0] = '\0';
        hours[0] = in_charge[0] = qr_url[0]  = '\0';
        for (int i = 0; i < MAX_TAGS; i++) tags[i][0] = '\0';
    }
};

// Linked list wrapper
struct BuildingList {
    Building* head;
    int       size;

    BuildingList() : head(nullptr), size(0) {}
    ~BuildingList();

    void     append(Building* b);
    Building* findById(int id) const;
    void     printAll() const;
    void     clear();
};

// Public API
BuildingList* loadBuildings(const char* csvPath);
bool          saveBuildings(const BuildingList* list, const char* csvPath);
bool          updateBuilding(BuildingList* list, int id, float x, float y, const char* name);
std::vector<Building*> searchBuildings(const BuildingList* list, const char* query);
void          printBuildingDetail(const Building* b);
