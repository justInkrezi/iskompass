// buildings.cpp — Building linked list for ISKOMPASS
#include "buildings.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cstring>

// ── BuildingList destructor ─────────────────────
BuildingList::~BuildingList() { clear(); }

void BuildingList::clear() {
    Building* cur = head;
    while (cur) {
        Building* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
    head = nullptr;
    size = 0;
}

// ── Append node to end of list ──────────────────
void BuildingList::append(Building* b) {
    b->next = nullptr;
    if (!head) { head = b; size = 1; return; }
    Building* cur = head;
    while (cur->next) cur = cur->next;
    cur->next = b;
    size++;
}

// ── Linear search by ID ─────────────────────────
Building* BuildingList::findById(int id) const {
    Building* cur = head;
    while (cur) {
        if (cur->id == id) return cur;
        cur = cur->next;
    }
    return nullptr;
}

// ── Print all buildings ─────────────────────────
void BuildingList::printAll() const {
    Building* cur = head;
    while (cur) {
        printBuildingDetail(cur);
        cur = cur->next;
    }
}

// ── Helper: lowercase a string ──────────────────
static std::string toLower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

// ── Parse a CSV line respecting quotes ──────────
static std::vector<std::string> parseCSVLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuote = false;
    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == '"') {
            inQuote = !inQuote;
        } else if (c == ',' && !inQuote) {
            fields.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }
    fields.push_back(field);
    return fields;
}

// ── Load buildings from campus_data.csv ─────────
// CSV format:
// id,name,photo_path,services,hours,in_charge,tags,x,y,qr_url
// tags field is semicolon-separated (up to 5)
BuildingList* loadBuildings(const char* csvPath) {
    BuildingList* list = new BuildingList();
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        std::cerr << "[loadBuildings] Cannot open: " << csvPath << std::endl;
        return list;
    }

    std::string line;
    bool firstLine = true;
    while (std::getline(file, line)) {
        if (firstLine) { firstLine = false; continue; } // skip header
        if (line.empty()) continue;

        auto f = parseCSVLine(line);
        if (f.size() < 10) continue;

        Building* b = new Building();
        b->id = std::stoi(f[0]);
        strncpy(b->name,       f[1].c_str(), 99);
        strncpy(b->photo_path, f[2].c_str(), 199);
        strncpy(b->services,   f[3].c_str(), 299);
        strncpy(b->hours,      f[4].c_str(), 99);
        strncpy(b->in_charge,  f[5].c_str(), 99);

        // Parse semicolon-delimited tags
        std::istringstream tagStream(f[6]);
        std::string tag;
        int tagIdx = 0;
        while (std::getline(tagStream, tag, ';') && tagIdx < MAX_TAGS) {
            strncpy(b->tags[tagIdx++], tag.c_str(), 49);
        }

        b->x = std::stof(f[7]);
        b->y = std::stof(f[8]);
        strncpy(b->qr_url, f[9].c_str(), 299);

        list->append(b);
    }
    file.close();
    std::cout << "[loadBuildings] Loaded " << list->size << " buildings.\n";
    return list;
}

// ── Save buildings back to CSV ──────────────────
bool saveBuildings(const BuildingList* list, const char* csvPath) {
    std::ofstream file(csvPath);
    if (!file.is_open()) {
        std::cerr << "[saveBuildings] Cannot write: " << csvPath << std::endl;
        return false;
    }
    file << "id,name,photo_path,services,hours,in_charge,tags,x,y,qr_url\n";
    Building* cur = list->head;
    while (cur) {
        // Build tags string
        std::string tagsStr;
        for (int i = 0; i < MAX_TAGS; i++) {
            if (cur->tags[i][0] == '\0') break;
            if (!tagsStr.empty()) tagsStr += ';';
            tagsStr += cur->tags[i];
        }
        file << cur->id << ","
             << "\"" << cur->name       << "\","
             << "\"" << cur->photo_path << "\","
             << "\"" << cur->services   << "\","
             << "\"" << cur->hours      << "\","
             << "\"" << cur->in_charge  << "\","
             << "\"" << tagsStr         << "\","
             << cur->x << ","
             << cur->y << ","
             << "\"" << cur->qr_url     << "\"\n";
        cur = cur->next;
    }
    file.close();
    return true;
}

// ── Update building name and coordinates ────────
bool updateBuilding(BuildingList* list, int id, float x, float y, const char* name) {
    Building* b = list->findById(id);
    if (!b) { std::cerr << "[updateBuilding] ID not found: " << id << std::endl; return false; }
    if (name && strlen(name) > 0) strncpy(b->name, name, 99);
    b->x = x;
    b->y = y;
    return true;
}

// ── Linear search by keyword (name + tags) ──────
// Simulates C++ traversal of linked list
std::vector<Building*> searchBuildings(const BuildingList* list, const char* query) {
    std::vector<Building*> results;
    std::string q = toLower(query);
    Building* cur = list->head;
    while (cur) {
        bool match = false;
        if (toLower(cur->name).find(q) != std::string::npos) match = true;
        if (!match && toLower(cur->services).find(q) != std::string::npos) match = true;
        for (int i = 0; i < MAX_TAGS && !match; i++) {
            if (cur->tags[i][0] && toLower(cur->tags[i]).find(q) != std::string::npos)
                match = true;
        }
        if (match) results.push_back(cur);
        cur = cur->next;
    }
    return results;
}

// ── Pretty-print one building ───────────────────
void printBuildingDetail(const Building* b) {
    if (!b) return;
    std::cout << "─────────────────────────────────\n";
    std::cout << "  Building #" << b->id << ": " << b->name << "\n";
    std::cout << "  Services : " << b->services << "\n";
    std::cout << "  Hours    : " << b->hours << "\n";
    std::cout << "  In-Charge: " << b->in_charge << "\n";
    std::cout << "  Location : (" << b->x << ", " << b->y << ")\n";
    std::cout << "  Tags     : ";
    for (int i = 0; i < MAX_TAGS; i++) {
        if (b->tags[i][0] == '\0') break;
        std::cout << b->tags[i];
        if (i < MAX_TAGS-1 && b->tags[i+1][0]) std::cout << ", ";
    }
    std::cout << "\n";
    if (b->qr_url[0]) std::cout << "  QR URL   : " << b->qr_url << "\n";
    std::cout << "─────────────────────────────────\n";
}
