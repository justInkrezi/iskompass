// auth.cpp — Authentication for ISKOMPASS
#include "auth.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <ctime>

// ────────────────────────────────────────────────
//  protoHash — lightweight prototype hash
//  XOR-rotate over each character, formatted as hex.
//  NOTE: Replace with SHA-256 (OpenSSL) for production.
// ────────────────────────────────────────────────
std::string protoHash(const std::string& password) {
    unsigned long h = 5381UL;
    for (unsigned char c : password)
        h = ((h << 5) + h) ^ c;   // djb2
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << h;
    return oss.str();
}

// ────────────────────────────────────────────────
//  roleToStr / strToRole helpers
// ────────────────────────────────────────────────
static std::string roleToStr(UserRole r) {
    return (r == UserRole::ADMIN) ? "admin" : "student";
}

static UserRole strToRole(const std::string& s) {
    if (s == "admin") return UserRole::ADMIN;
    if (s == "student") return UserRole::STUDENT;
    return UserRole::UNKNOWN;
}

// ────────────────────────────────────────────────
//  usernameExists — linear scan of users.txt
// ────────────────────────────────────────────────
bool usernameExists(const char* filePath, const char* username) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string uname;
        std::getline(ss, uname, ',');
        if (uname == username) { file.close(); return true; }
    }
    file.close();
    return false;
}

// ────────────────────────────────────────────────
//  loginUser — find user + verify password hash
// ────────────────────────────────────────────────
User* loginUser(const char* filePath,
                const char* username,
                const char* plainPassword)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[loginUser] Cannot open users file: " << filePath << "\n";
        return nullptr;
    }

    std::string inputHash = protoHash(plainPassword);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        // Format: username,password_hash,full_name,role
        std::istringstream ss(line);
        std::string uname, phash, fname, role;

        if (!std::getline(ss, uname,  ',')) continue;
        if (!std::getline(ss, phash,  ',')) continue;
        if (!std::getline(ss, fname,  ',')) continue;
        if (!std::getline(ss, role))        continue;

        if (uname == username && phash == inputHash) {
            User* u = new User();
            strncpy(u->username,      uname.c_str(), 63);
            strncpy(u->password_hash, phash.c_str(), 127);
            strncpy(u->full_name,     fname.c_str(), 99);
            u->role = strToRole(role);
            file.close();
            std::cout << "[loginUser] Authenticated: " << u->username
                      << " (" << role << ")\n";
            return u;
        }
    }
    file.close();
    std::cerr << "[loginUser] Authentication failed for: " << username << "\n";
    return nullptr;
}

// ────────────────────────────────────────────────
//  signupUser — append new user to users.txt
// ────────────────────────────────────────────────
bool signupUser(const char* filePath,
                const char* username,
                const char* plainPassword,
                const char* fullName,
                UserRole    role)
{
    // Validate input
    if (!username || strlen(username) < 3) {
        std::cerr << "[signupUser] Username too short.\n";
        return false;
    }
    if (!plainPassword || strlen(plainPassword) < 6) {
        std::cerr << "[signupUser] Password must be at least 6 characters.\n";
        return false;
    }

    // Check for duplicate username
    if (usernameExists(filePath, username)) {
        std::cerr << "[signupUser] Username already taken: " << username << "\n";
        return false;
    }

    // Append to file
    std::ofstream file(filePath, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "[signupUser] Cannot open users file for writing: "
                  << filePath << "\n";
        return false;
    }

    std::string hash = protoHash(plainPassword);
    file << username << ","
         << hash     << ","
         << fullName << ","
         << roleToStr(role) << "\n";

    file.close();
    std::cout << "[signupUser] New user created: " << username << "\n";
    return true;
}

// ────────────────────────────────────────────────
//  printUser — debug
// ────────────────────────────────────────────────
void printUser(const User* u) {
    if (!u) { std::cout << "  (null user)\n"; return; }
    std::cout << "  User     : " << u->username  << "\n"
              << "  Name     : " << u->full_name << "\n"
              << "  Role     : " << roleToStr(u->role) << "\n";
}
