# 🧭 ISKOMPASS — UP Cebu Campus Guide (Prototype)

> Mobile-first campus companion for UP Cebu students.
> Navigate buildings, find jeepney routes, and submit grievances via QR.

---

## Project Structure

```
iskompass/
├── frontend/
│   ├── index.html        ← All 7 screens in one shell (SPA)
│   ├── style.css         ← Full mobile-first stylesheet
│   └── app.js            ← UI logic + Dijkstra (JS prototype)
│
├── backend/
│   ├── main.cpp          ← Entry point; command-line REPL router
│   ├── buildings.cpp/h   ← Singly linked list of Building nodes
│   ├── graph.cpp/h       ← Adjacency list + Dijkstra's algorithm
│   ├── favorites.cpp/h   ← Per-user favorites linked list
│   ├── auth.cpp/h        ← Login / signup / password hashing
│   ├── admin.cpp/h       ← Data editor, route manager, log writer
│   ├── AdminMode.cpp     ← Standalone admin CLI utility
│   └── Makefile
│
└── data/
    ├── campus_data.csv   ← Building records (14 buildings)
    ├── routes_data.csv   ← Jeepney stop graph edges
    ├── favorites.txt     ← Per-user saved building IDs
    ├── users.txt         ← User credentials (hashed)
    └── admin_log.txt     ← Timestamped admin action log
```

---

## Quick Start

### Frontend (HTML prototype — works in any browser)
```bash
cd frontend
# Open index.html in your browser, or serve it:
python3 -m http.server 8080
# Then visit http://localhost:8080
```

**Demo credentials:**
| Username     | Password   | Role    |
|-------------|------------|---------|
| user         | password   | Student |
| iskostudent  | isko2025   | Student |
| admin        | admin123   | Admin   |

---

### Backend (C++)
```bash
cd backend
make all          # builds iskompass + adminmode binaries
make run          # launches the REPL
```

**REPL commands:**
```
login    user password
signup   newuser newpass "Full Name"
buildings                         — list all 14 buildings
building 3                        — detail for Building #3
search   food                     — find buildings tagged "food"
route    Colon_Street UP_Cebu_Entrance_Gate
route    Colon_Street UP_Cebu_Entrance_Gate traffic
favorites
fav-add  4
fav-rm   4
admin-bldg 2 "Science_Building" 0.46 0.23
admin-route 04L 30
log
quit
```

**AdminMode (standalone):**
```bash
./bin/adminmode                          # interactive menu
./bin/adminmode bldg 2 "COS" 0.46 0.23  # CLI update
./bin/adminmode route 04L 30
./bin/adminmode log 20
```

---

## Data Structures Used

| Module     | Structure              | Why                                   |
|-----------|------------------------|---------------------------------------|
| Buildings  | Singly linked list     | Dynamic insertion, O(n) search        |
| Favorites  | Singly linked list     | Ordered, per-user, file-backed        |
| Jeepney    | Adjacency list (graph) | Sparse graph, route-filtering support |
| Dijkstra   | Linear-scan min queue  | Correct for small campus graph        |

---

## Screens

| Screen | Description                             |
|--------|-----------------------------------------|
| 1      | Login / Sign Up                         |
| 2      | Home — feature cards + campus notice    |
| 3      | Navigate — 2D map + building list + detail slide-up |
| 4      | Jeepney Routes — Dijkstra path finder   |
| 5      | QR Grievance — office cards + QR modal  |
| 6      | Favorites — saved buildings             |
| 7      | Profile / Settings + hidden Admin panel |

---

## Known Prototype Limitations

- **Password hashing** uses djb2 (prototype only). Replace with SHA-256 via OpenSSL in production.
- **Map** is a static SVG placeholder. Replace with an actual OpenStreetMap PNG or tile layer.
- **Building photos** are emoji placeholders. Replace `photo_path` values in `campus_data.csv` with real image paths.
- **QR scanning** opens a modal with a placeholder. Integrate a JS QR library (e.g. `jsQR`) or native camera API for real scanning.
- **Backend ↔ Frontend** communication is simulated in `app.js`. In production, wire up via a lightweight C++ HTTP server (e.g. `cpp-httplib`) or CGI.
- **Traffic multiplier** (1.3×) is hardcoded. Expose as an admin-configurable setting in a future version.

---

## File Formats

### campus_data.csv
```
id,name,photo_path,services,hours,in_charge,tags,x,y,qr_url
1,"Admin Building","img/admin.jpg","Registrar...","Mon-Fri 8-5","Chancellor","enrollment;admin",0.25,0.28,""
```
Tags are semicolon-separated; max 5 per building.

### routes_data.csv
```
from_stop,to_stop,base_time,route,bidirectional
"Colon Street","Carbon Market",5,04L,1
```
`bidirectional=1` adds the reverse edge automatically on load.

### users.txt
```
username,password_hash,full_name,role
user,<djb2_hex>,Isko Student,student
```

### favorites.txt
```
username,buildingId
iskostudent,4
iskostudent,12
```

### admin_log.txt
```
[YYYY-MM-DD HH:MM:SS] admin=<user> action=<action> detail=<detail>
```

---

*Built for UP Cebu — Gabay Isko, June 2025*
