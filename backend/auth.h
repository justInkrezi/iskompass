#pragma once
// auth.h — Login / signup logic for ISKOMPASS
// Persistence: users.txt
// Format per line: username,password_hash,full_name,role
// Role: "student" | "admin"

#include <string>

enum class UserRole { STUDENT, ADMIN, UNKNOWN };

struct User {
    char     username[64];
    char     password_hash[128]; // SHA-256 hex or plaintext for prototype
    char     full_name[100];
    UserRole role;

    User() : role(UserRole::UNKNOWN) {
        username[0] = password_hash[0] = full_name[0] = '\0';
    }
};

// ── Public API ───────────────────────────────────
// Returns allocated User on success, nullptr on failure.
// Caller must delete the returned pointer.
User* loginUser(const char* filePath,
                const char* username,
                const char* plainPassword);

bool  signupUser(const char* filePath,
                 const char* username,
                 const char* plainPassword,
                 const char* fullName,
                 UserRole    role = UserRole::STUDENT);

bool  usernameExists(const char* filePath, const char* username);

// Simple prototype hash: XOR + rotate (NOT cryptographically secure)
// Replace with SHA-256 via OpenSSL in production.
std::string protoHash(const std::string& password);

void  printUser(const User* u);
