# Makefile — ISKOMPASS C++ Backend
# Usage:
#   make          — build main backend binary
#   make admin    — build standalone admin utility
#   make all      — build both
#   make clean    — remove build artifacts
#   make run      — run main backend REPL

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SRC_DIR  = .
BIN_DIR  = ../bin

SRCS     = buildings.cpp graph.cpp favorites.cpp auth.cpp admin.cpp main.cpp
ADMIN_SRCS = buildings.cpp graph.cpp favorites.cpp auth.cpp admin.cpp AdminMode.cpp

OBJS     = $(SRCS:.cpp=.o)
ADMIN_OBJS = $(ADMIN_SRCS:.cpp=.o)

TARGET      = $(BIN_DIR)/iskompass
ADMIN_TARGET = $(BIN_DIR)/adminmode

# ── Default: build main backend ──────────────────
all: dirs $(TARGET) $(ADMIN_TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Built: $@"

# ── Admin utility ─────────────────────────────────
admin: dirs $(ADMIN_TARGET)

$(ADMIN_TARGET): $(ADMIN_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Built: $@"

# ── Compile rules ─────────────────────────────────
%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ── Create bin dir ────────────────────────────────
dirs:
	mkdir -p $(BIN_DIR)

# ── Run REPL ─────────────────────────────────────
run: $(TARGET)
	$(TARGET)

# ── Cleanup ───────────────────────────────────────
clean:
	rm -f *.o $(TARGET) $(ADMIN_TARGET)
	@echo "Cleaned."

.PHONY: all admin dirs run clean
