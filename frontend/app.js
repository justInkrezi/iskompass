// ═══════════════════════════════════════════════
//  ISKOMPASS — app.js
//  UI logic, state management, mock backend calls
// ═══════════════════════════════════════════════

'use strict';

// ── App State ──────────────────────────────────
const state = {
  currentUser: null,
  isAdmin: false,
  favorites: JSON.parse(localStorage.getItem('isko_favorites') || '[]'),
  darkMode: localStorage.getItem('isko_dark') === 'true',
  direction: 'to',       // 'to' | 'from'
  activeBuilding: null,
  activeRoute: 'all',
};

// ── Mock User Database (users.txt simulation) ──
const USERS = [
  { username: 'user',       password: 'password', name: 'Isko Student',   role: 'student' },
  { username: 'iskostudent',password: 'isko2025', name: 'Maria Santos',   role: 'student' },
  { username: 'admin',      password: 'admin123', name: 'Campus Admin',   role: 'admin'   },
];

// ── Campus Data (campus_data.csv simulation) ───
// C++ struct mirrors: id, name, photo_path, services, hours, in_charge, tags[5], x, y, qr_url
const BUILDINGS = [
  {
    id: 1, name: 'Administration Building', emoji: '🏛️',
    services: 'Office of the Chancellor, Registrar, Finance, Admissions, Human Resources',
    hours: 'Mon–Fri, 8:00 AM – 5:00 PM',
    inCharge: 'Office of the Chancellor',
    tags: ['enrollment', 'admin', 'finance'],
    x: 0.25, y: 0.28, qr_url: 'https://forms.google.com/admin'
  },
  {
    id: 2, name: 'Science Building (COS)', emoji: '🔬',
    services: 'Biology Lab, Chemistry Lab, Physics Lab, Math Dept, Computer Lab',
    hours: 'Mon–Fri, 7:00 AM – 6:00 PM',
    inCharge: 'Dean of College of Science',
    tags: ['science', 'lab', 'wifi'],
    x: 0.45, y: 0.22, qr_url: ''
  },
  {
    id: 3, name: 'Technology Innovation Center', emoji: '💡',
    services: 'Research Hub, Co-Working Spaces, Innovation Labs, 3D Printing',
    hours: 'Mon–Fri, 8:00 AM – 7:00 PM',
    inCharge: 'TIC Director',
    tags: ['wifi', 'research', 'printing'],
    x: 0.65, y: 0.30, qr_url: ''
  },
  {
    id: 4, name: 'Library Building', emoji: '📚',
    services: 'Main Library, Reading Rooms, Digital Archives, Printing Services',
    hours: 'Mon–Sat, 7:30 AM – 7:00 PM',
    inCharge: 'University Librarian',
    tags: ['library', 'printing', 'wifi', 'study'],
    x: 0.38, y: 0.45, qr_url: ''
  },
  {
    id: 5, name: 'Residence Halls (Lihangin/Varangao/Liadlaw/Lalahon)', emoji: '🏠',
    services: 'Student Dormitories, Dormitory Office, Common Areas',
    hours: 'Open 24 hours (Dormitory Office: 8AM–5PM)',
    inCharge: 'Dormitory Manager',
    tags: ['housing', 'residence', 'dorm'],
    x: 0.20, y: 0.55, qr_url: ''
  },
  {
    id: 6, name: 'School of Management', emoji: '📊',
    services: 'SOM Admin, Classrooms, Faculty Offices, Student Lounge',
    hours: 'Mon–Fri, 7:30 AM – 5:30 PM',
    inCharge: 'Dean of School of Management',
    tags: ['management', 'admin', 'enrollment'],
    x: 0.72, y: 0.55, qr_url: ''
  },
  {
    id: 7, name: 'Undergraduate Building', emoji: '🏫',
    services: 'General Classrooms, Faculty Offices, Study Halls',
    hours: 'Mon–Fri, 7:00 AM – 6:00 PM',
    inCharge: 'Registrar Office',
    tags: ['classes', 'enrollment', 'faculty'],
    x: 0.55, y: 0.65, qr_url: ''
  },
  {
    id: 8, name: 'Arts & Design Workshop', emoji: '🎨',
    services: 'Visual Arts Studio, Design Workshop, Exhibition Area',
    hours: 'Mon–Fri, 8:00 AM – 6:00 PM',
    inCharge: 'Arts & Design Department Head',
    tags: ['arts', 'design', 'workshop'],
    x: 0.35, y: 0.72, qr_url: ''
  },
  {
    id: 9, name: 'Social Sciences Building', emoji: '🌐',
    services: 'Sociology Dept, Political Science, History, Social Work Offices',
    hours: 'Mon–Fri, 8:00 AM – 5:00 PM',
    inCharge: 'Dean, College of Social Sciences',
    tags: ['social', 'classes', 'faculty'],
    x: 0.50, y: 0.80, qr_url: ''
  },
  {
    id: 11, name: 'AS Conference Hall / AS East Wing', emoji: '🎙️',
    services: 'Conference Room, Seminar Halls, Events Venue',
    hours: 'Mon–Fri, 7:00 AM – 9:00 PM (by reservation)',
    inCharge: 'Administrative Services',
    tags: ['events', 'conference', 'admin'],
    x: 0.60, y: 0.40, qr_url: ''
  },
  {
    id: 12, name: 'Union Building', emoji: '☕',
    services: 'Cafeteria, Student Organizations Hub, UPCSG Office, Photocopy Center',
    hours: 'Mon–Sat, 7:00 AM – 8:00 PM',
    inCharge: 'Student Council',
    tags: ['food', 'printing', 'student', 'wifi'],
    x: 0.28, y: 0.62, qr_url: ''
  },
  {
    id: 13, name: 'UP Cebu Go Negosyo Center', emoji: '💼',
    services: 'Business Registration, MSME Support, Livelihood Programs, DTI Services',
    hours: 'Mon–Fri, 8:00 AM – 5:00 PM',
    inCharge: 'Go Negosyo Center Coordinator',
    tags: ['business', 'services', 'government'],
    x: 0.75, y: 0.72, qr_url: 'https://forms.google.com/gonegosyo'
  },
  {
    id: 14, name: 'Co-Working Space', emoji: '💻',
    services: 'Open Co-Working Desks, Meeting Pods, High-Speed WiFi, Printing',
    hours: 'Mon–Fri, 8:00 AM – 8:00 PM; Sat, 9:00 AM – 5:00 PM',
    inCharge: 'TIC Facilities Manager',
    tags: ['wifi', 'study', 'printing', 'coworking'],
    x: 0.42, y: 0.35, qr_url: ''
  },
];

// ── Jeepney Graph (routes_data.csv simulation) ──
// Adjacency list: each stop -> neighbors with time & route
const JEEPNEY_GRAPH = {
  'Colon Street':         [{ dest: 'Carbon Market', time: 5, route: '04L' }, { dest: 'SM City Cebu', time: 12, route: '12L' }],
  'Carbon Market':        [{ dest: 'Colon Street', time: 5, route: '04L' }, { dest: 'Fuente Osmena', time: 8, route: '04L' }],
  'Fuente Osmena':        [{ dest: 'Carbon Market', time: 8, route: '04L' }, { dest: 'Lahug Area', time: 10, route: '04L' }, { dest: 'IT Park', time: 8, route: '01C' }],
  'Lahug Area':           [{ dest: 'Fuente Osmena', time: 10, route: '04L' }, { dest: 'Talamban Junction', time: 12, route: '04L' }, { dest: 'Gaisano Country Mall', time: 7, route: '20B' }],
  'Talamban Junction':    [{ dest: 'Lahug Area', time: 12, route: '04L' }, { dest: 'UP Cebu Entrance Gate', time: 5, route: '04L' }, { dest: 'Gaisano Country Mall', time: 4, route: '20B' }],
  'UP Cebu Entrance Gate':[{ dest: 'Talamban Junction', time: 5, route: '04L' }],
  'SM City Cebu':         [{ dest: 'Colon Street', time: 12, route: '12L' }, { dest: 'IT Park', time: 6, route: '12L' }, { dest: 'Ayala Center Cebu', time: 10, route: '12L' }],
  'Ayala Center Cebu':    [{ dest: 'SM City Cebu', time: 10, route: '12L' }, { dest: 'IT Park', time: 8, route: '10C' }, { dest: 'Talamban Junction', time: 18, route: '10C' }],
  'IT Park':              [{ dest: 'Fuente Osmena', time: 8, route: '01C' }, { dest: 'Ayala Center Cebu', time: 8, route: '10C' }, { dest: 'SM City Cebu', time: 6, route: '12L' }],
  'Gaisano Country Mall': [{ dest: 'Lahug Area', time: 7, route: '20B' }, { dest: 'Talamban Junction', time: 4, route: '20B' }],
  'North Bus Terminal':   [{ dest: 'Colon Street', time: 10, route: '01C' }, { dest: 'SM City Cebu', time: 15, route: '01C' }],
};

// ── QR Office Data ─────────────────────────────
const QR_OFFICES = [
  { name: 'Office of the Chancellor', emoji: '🏛️', desc: 'Major concerns & appeals', url: 'https://forms.google.com/chancellor' },
  { name: 'Office of the Registrar', emoji: '📋', desc: 'Enrollment & documents', url: 'https://forms.google.com/registrar' },
  { name: 'Office of Student Affairs', emoji: '🎓', desc: 'Student welfare & concerns', url: 'https://forms.google.com/osa' },
  { name: 'Library', emoji: '📚', desc: 'Library feedback & requests', url: 'https://forms.google.com/library' },
  { name: 'Health Services Unit', emoji: '🏥', desc: 'Health concerns & reports', url: 'https://forms.google.com/health' },
  { name: 'Security Office', emoji: '🛡️', desc: 'Safety & security incidents', url: 'https://forms.google.com/security' },
  { name: 'UPCSG (Student Council)', emoji: '🤝', desc: 'Student org concerns', url: 'https://forms.google.com/upcsg' },
  { name: 'Dormitory Office', emoji: '🏠', desc: 'Residence hall concerns', url: 'https://forms.google.com/dorm' },
];

// ═══════════════════════════════════════════════
//  AUTH
// ═══════════════════════════════════════════════

function doLogin() {
  const uname = document.getElementById('login-user').value.trim();
  const pass  = document.getElementById('login-pass').value;
  const errEl = document.getElementById('login-error');

  const found = USERS.find(u => u.username === uname && u.password === pass);
  if (!found) {
    errEl.classList.add('show');
    return;
  }
  errEl.classList.remove('show');
  state.currentUser = found;
  state.isAdmin = found.role === 'admin';

  document.getElementById('home-greeting').textContent =
    `Hello, ${found.name.split(' ')[0]}! Your UP Cebu Companion.`;

  // Update profile
  document.getElementById('profile-name').textContent = found.name;
  document.getElementById('profile-username').textContent = '@' + found.username;
  document.getElementById('profile-role').textContent = found.role === 'admin' ? '🛡️ Admin' : '🎓 Student';
  document.getElementById('profile-avatar').textContent = found.name.split(' ').map(n=>n[0]).join('').slice(0,2).toUpperCase();

  document.getElementById('tab-bar').style.display = 'flex';
  goTo('home');
}

function showSignup() {
  showToast('Sign up coming soon! Use demo: user / password');
}

function doLogout() {
  state.currentUser = null;
  state.isAdmin = false;
  document.getElementById('tab-bar').style.display = 'none';
  document.getElementById('admin-section').classList.remove('show');
  document.getElementById('login-user').value = '';
  document.getElementById('login-pass').value = '';
  goTo('login');
}

// ═══════════════════════════════════════════════
//  NAVIGATION
// ═══════════════════════════════════════════════

function goTo(screen) {
  document.querySelectorAll('.screen').forEach(s => s.classList.remove('active'));
  document.getElementById('screen-' + screen).classList.add('active');

  document.querySelectorAll('.tab-item').forEach(t => t.classList.remove('active'));
  const tab = document.getElementById('tab-' + screen);
  if (tab) tab.classList.add('active');

  if (screen === 'favorites') renderFavorites();
  if (screen === 'navigate') renderBuildings(BUILDINGS);
  if (screen === 'qr')       renderQRGrid();
}

// ═══════════════════════════════════════════════
//  NAVIGATE / BUILDINGS
// ═══════════════════════════════════════════════

function renderBuildings(buildings) {
  const container = document.getElementById('buildings-container');
  if (!buildings.length) {
    container.innerHTML = '<p style="text-align:center;color:var(--gray-400);font-size:13px;padding:20px 0;">No buildings found.</p>';
    return;
  }
  container.innerHTML = buildings.map(b => `
    <div class="building-item" onclick="openBuilding(${BUILDINGS.indexOf(b)})">
      <div class="building-num">${b.id}</div>
      <div class="building-info">
        <h4>${b.name}</h4>
        <p>${b.inCharge}</p>
        <div class="building-tags">
          ${b.tags.map(t => `<span class="tag-chip">${t}</span>`).join('')}
        </div>
      </div>
      <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="var(--gray-400)" stroke-width="2"><path d="m9 18 6-6-6-6"/></svg>
    </div>
  `).join('');
}

function searchBuildings(query) {
  const q = query.toLowerCase().trim();
  if (!q) { renderBuildings(BUILDINGS); return; }
  // Linear traversal of linked list (simulated)
  const results = BUILDINGS.filter(b =>
    b.name.toLowerCase().includes(q) ||
    b.tags.some(t => t.toLowerCase().includes(q)) ||
    b.services.toLowerCase().includes(q)
  );
  renderBuildings(results);
}

function openBuilding(index) {
  const b = BUILDINGS[index];
  state.activeBuilding = b;

  document.getElementById('detail-photo').textContent = b.emoji;
  document.getElementById('detail-name').textContent = b.name;
  document.getElementById('detail-id-badge').textContent = `Building #${b.id}`;
  document.getElementById('detail-services').textContent = b.services;
  document.getElementById('detail-hours').textContent = b.hours;
  document.getElementById('detail-incharge').textContent = b.inCharge;

  // Tags
  document.getElementById('detail-tags').innerHTML = b.tags.map(t =>
    `<span class="tag-chip">${t}</span>`
  ).join('');

  // QR button
  const qrBtn = document.getElementById('detail-qr-btn');
  qrBtn.style.display = b.qr_url ? 'flex' : 'none';

  // Favorite button
  const isFav = state.favorites.includes(b.id);
  const favBtn = document.getElementById('detail-fav-btn');
  favBtn.textContent = isFav ? '★ Saved' : '☆ Save';
  favBtn.classList.toggle('saved', isFav);

  document.getElementById('detail-overlay').classList.add('open');
}

function closeDetail(event) {
  if (!event || event.target === document.getElementById('detail-overlay')) {
    document.getElementById('detail-overlay').classList.remove('open');
    state.activeBuilding = null;
  }
}

function toggleFavorite() {
  if (!state.activeBuilding) return;
  const id = state.activeBuilding.id;
  const idx = state.favorites.indexOf(id);

  if (idx === -1) {
    state.favorites.push(id);
    showToast(`★ ${state.activeBuilding.name} saved!`);
  } else {
    state.favorites.splice(idx, 1);
    showToast(`Removed from favorites`);
  }

  localStorage.setItem('isko_favorites', JSON.stringify(state.favorites));

  const isFav = state.favorites.includes(id);
  const favBtn = document.getElementById('detail-fav-btn');
  favBtn.textContent = isFav ? '★ Saved' : '☆ Save';
  favBtn.classList.toggle('saved', isFav);
}

// ═══════════════════════════════════════════════
//  JEEPNEY ROUTES — Dijkstra's Algorithm (JS)
// ═══════════════════════════════════════════════

function setDirection(dir) {
  state.direction = dir;
  document.getElementById('toggle-to').classList.toggle('active', dir === 'to');
  document.getElementById('toggle-from').classList.toggle('active', dir === 'from');

  const startLabel = document.getElementById('start-label');
  const destLabel  = document.getElementById('dest-label');
  const destSel    = document.getElementById('dest-stop');

  if (dir === 'to') {
    startLabel.textContent = '📍 Start Location';
    destLabel.textContent  = '🏁 Destination';
    // Ensure UP Cebu is first option in dest
    if (!destSel.options[1]?.value.includes('UP')) {
      destSel.innerHTML = `
        <option value="">Select destination...</option>
        <option>UP Cebu Entrance Gate</option>
        <option>Colon Street</option>
        <option>SM City Cebu</option>
        <option>Ayala Center Cebu</option>
        <option>Carbon Market</option>
        <option>Fuente Osmena</option>
        <option>Talamban Junction</option>
      `;
    }
  } else {
    startLabel.textContent = '📍 Start (UP Cebu)';
    destLabel.textContent  = '🏁 Where to?';
    document.getElementById('start-stop').value = '';
    destSel.innerHTML = `
      <option value="">Select destination...</option>
      <option>Colon Street</option>
      <option>SM City Cebu</option>
      <option>Ayala Center Cebu</option>
      <option>Carbon Market</option>
      <option>Fuente Osmena</option>
      <option>IT Park</option>
      <option>Gaisano Country Mall</option>
    `;
  }
  document.getElementById('route-result').classList.remove('show');
}

function filterRoute(btn, route) {
  document.querySelectorAll('.route-pill').forEach(p => p.classList.remove('active'));
  btn.classList.add('active');
  state.activeRoute = route;
}

// Dijkstra's shortest path
function dijkstra(graph, start, end) {
  const dist  = {};
  const prev  = {};
  const prevRoute = {};
  const visited = new Set();
  const nodes = Object.keys(graph);

  nodes.forEach(n => { dist[n] = Infinity; prev[n] = null; });
  dist[start] = 0;

  while (true) {
    // Pick unvisited node with min dist
    let u = null;
    nodes.forEach(n => {
      if (!visited.has(n) && dist[n] !== Infinity) {
        if (u === null || dist[n] < dist[u]) u = n;
      }
    });
    if (u === null || u === end) break;
    visited.add(u);

    const neighbors = graph[u] || [];
    neighbors.forEach(({ dest, time, route }) => {
      if (!graph[dest]) return;
      // Filter by active route
      if (state.activeRoute !== 'all' && route !== state.activeRoute) return;
      const alt = dist[u] + time;
      if (alt < dist[dest]) {
        dist[dest] = alt;
        prev[dest] = u;
        prevRoute[dest] = route;
      }
    });
  }

  if (dist[end] === Infinity) return null;

  // Reconstruct path
  const path = [];
  let cur = end;
  while (cur) {
    path.unshift({ stop: cur, route: prevRoute[cur] || null });
    cur = prev[cur];
  }
  return { path, totalTime: dist[end] };
}

function findRoute() {
  const start = document.getElementById('start-stop').value;
  const dest  = document.getElementById('dest-stop').value;

  if (!start || !dest) {
    showToast('⚠️ Please select both start and destination.');
    return;
  }
  if (start === dest) {
    showToast('⚠️ Start and destination are the same.');
    return;
  }

  const useTraffic = document.getElementById('traffic-toggle').checked;
  const TRAFFIC_MULTIPLIER = 1.3;

  // If direction is "from UP Cebu", swap
  const actualStart = state.direction === 'from' ? 'UP Cebu Entrance Gate' : start;
  const actualEnd   = state.direction === 'from' ? dest : dest;

  const result = dijkstra(JEEPNEY_GRAPH, actualStart, actualEnd);
  const resultEl = document.getElementById('route-result');

  if (!result) {
    showToast('No route found. Try a different combination.');
    resultEl.classList.remove('show');
    return;
  }

  let baseTime = result.totalTime;
  let dispTime = useTraffic ? Math.round(baseTime * TRAFFIC_MULTIPLIER) : baseTime;
  let minTime  = dispTime;
  let maxTime  = Math.round(dispTime * 1.4);

  // Estimate fare: base ₱15 + ₱1.5 per extra stop over 4
  const stopCount = result.path.length;
  const fare = Math.max(15, 15 + (stopCount - 2) * 2);

  // Determine primary route
  const routes = result.path.map(p => p.route).filter(Boolean);
  const primaryRoute = routes[0] || '04L';

  document.getElementById('result-route-name').textContent = `${actualStart} → ${actualEnd}`;
  document.getElementById('result-route-code').textContent = `via ${primaryRoute}${useTraffic ? ' (with traffic)' : ''}`;
  document.getElementById('result-time').textContent = `${minTime}–${maxTime} min`;
  document.getElementById('result-fare').textContent = `₱${fare}`;
  document.getElementById('result-range').textContent = `${minTime}–${maxTime} min`;

  // Build steps
  const stepsHtml = result.path.map((item, i) => {
    const isFirst = i === 0;
    const isLast  = i === result.path.length - 1;
    const dotClass = isLast ? 'dest' : (item.stop.includes('walk') ? 'walk' : '');
    const icon = isLast ? '🏁' : (isFirst ? '📍' : item.route || '🚌');
    return `
      <div class="route-step">
        <div class="step-dot ${dotClass}">${i + 1}</div>
        <div class="step-info">
          <h4>${item.stop}</h4>
          <p>${item.route ? `Ride ${item.route}` : (isFirst ? 'Starting point' : 'Destination')}</p>
        </div>
      </div>
    `;
  }).join('');

  // Add walk from gate
  const walkStep = `
    <div class="route-step">
      <div class="step-dot walk">🚶</div>
      <div class="step-info">
        <h4>Walk ~3 min</h4>
        <p>From Entrance Gate to destination building</p>
      </div>
    </div>
  `;

  document.getElementById('route-steps').innerHTML = stepsHtml + (actualEnd === 'UP Cebu Entrance Gate' ? walkStep : '');
  resultEl.classList.add('show');
}

// ═══════════════════════════════════════════════
//  QR GRIEVANCE
// ═══════════════════════════════════════════════

function renderQRGrid() {
  const grid = document.getElementById('qr-grid');
  if (grid.children.length) return; // already rendered
  grid.innerHTML = QR_OFFICES.map((o, i) => `
    <div class="qr-card" onclick="openQRModal(${i})">
      <div class="qr-card-icon">${o.emoji}</div>
      <h3>${o.name}</h3>
      <p>${o.desc}</p>
      <span class="qr-scan-badge">📷 Scan QR</span>
    </div>
  `).join('');
}

function openQRModal(index) {
  const o = QR_OFFICES[index];
  document.getElementById('qr-modal-title').textContent = o.name;
  document.getElementById('qr-modal-desc').textContent = 'Scan to open the grievance form for this office.';
  document.getElementById('qr-modal-office').textContent = o.name;
  document.getElementById('qr-modal-overlay').classList.add('open');
}

function closeQRModal(event) {
  if (!event || event.target === document.getElementById('qr-modal-overlay')) {
    document.getElementById('qr-modal-overlay').classList.remove('open');
  }
}

function openQRPage() {
  closeDetail();
  goTo('qr');
}

// ═══════════════════════════════════════════════
//  FAVORITES
// ═══════════════════════════════════════════════

function renderFavorites() {
  const list  = document.getElementById('favorites-list');
  const empty = document.getElementById('fav-empty');

  const favBuildings = BUILDINGS.filter(b => state.favorites.includes(b.id));

  if (!favBuildings.length) {
    empty.classList.add('show');
    list.innerHTML = '';
    return;
  }
  empty.classList.remove('show');

  list.innerHTML = favBuildings.map(b => `
    <div class="fav-item" id="fav-item-${b.id}">
      <div class="fav-thumb">${b.emoji}</div>
      <div class="fav-info">
        <h4>${b.name}</h4>
        <p>${b.hours}</p>
      </div>
      <div class="fav-actions">
        <button class="fav-btn view" onclick="openBuildingFromFav(${BUILDINGS.indexOf(b)})">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="m9 18 6-6-6-6"/></svg>
        </button>
        <button class="fav-btn remove" onclick="removeFavorite(${b.id})">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"/><path d="m19 6-1 14a2 2 0 0 1-2 2H8a2 2 0 0 1-2-2L5 6m5 0V4a1 1 0 0 1 1-1h2a1 1 0 0 1 1 1v2"/></svg>
        </button>
      </div>
    </div>
  `).join('');
}

function openBuildingFromFav(index) {
  goTo('navigate');
  setTimeout(() => openBuilding(index), 300);
}

function removeFavorite(id) {
  const idx = state.favorites.indexOf(id);
  if (idx !== -1) state.favorites.splice(idx, 1);
  localStorage.setItem('isko_favorites', JSON.stringify(state.favorites));
  showToast('Removed from favorites');
  renderFavorites();
}

// ═══════════════════════════════════════════════
//  PROFILE / SETTINGS
// ═══════════════════════════════════════════════

function toggleDarkMode() {
  state.darkMode = document.getElementById('dark-toggle').checked;
  document.body.classList.toggle('dark', state.darkMode);
  document.getElementById('dark-mode-label').textContent =
    state.darkMode ? 'Currently: Dark' : 'Currently: Light';
  localStorage.setItem('isko_dark', state.darkMode);
}

function clearCache() {
  localStorage.removeItem('isko_favorites');
  state.favorites = [];
  showToast('🗑️ Cache cleared');
}

function showAdminLogin() {
  if (state.isAdmin) {
    document.getElementById('admin-section').classList.toggle('show');
    return;
  }
  const pass = prompt('Enter admin password:');
  if (pass === 'admin123') {
    state.isAdmin = true;
    document.getElementById('admin-section').classList.add('show');
    showToast('🛡️ Admin access granted');
  } else {
    showToast('❌ Wrong admin password');
  }
}

function adminSaveBuilding() {
  const name = document.getElementById('admin-bldg-name').value;
  const x    = document.getElementById('admin-bldg-x').value;
  const y    = document.getElementById('admin-bldg-y').value;
  if (!name || !x || !y) { showToast('⚠️ Fill all fields'); return; }
  const log = document.getElementById('admin-log-box');
  const now = new Date().toLocaleString();
  log.innerHTML += `\nBuilding "${name}" updated [${now}]`;
  showToast(`✅ Building data saved (simulated)`);
  // In production: POST to C++ backend → writes campus_data.csv
}

function adminSaveRoute() {
  const code = document.getElementById('admin-route-code').value;
  const time = document.getElementById('admin-route-time').value;
  if (!code || !time) { showToast('⚠️ Fill all fields'); return; }
  const log = document.getElementById('admin-log-box');
  const now = new Date().toLocaleString();
  log.innerHTML += `\nRoute ${code} updated: base_time=${time}min [${now}]`;
  showToast(`✅ Route ${code} saved (simulated)`);
  // In production: POST to C++ backend → writes routes_data.csv
}

// ═══════════════════════════════════════════════
//  TOAST NOTIFICATION
// ═══════════════════════════════════════════════

let toastTimer = null;
function showToast(msg) {
  const t = document.getElementById('toast');
  t.textContent = msg;
  t.classList.add('show');
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => t.classList.remove('show'), 2800);
}

// ═══════════════════════════════════════════════
//  INIT
// ═══════════════════════════════════════════════

document.addEventListener('DOMContentLoaded', () => {
  // Apply saved dark mode
  if (state.darkMode) {
    document.body.classList.add('dark');
    document.getElementById('dark-toggle').checked = true;
    document.getElementById('dark-mode-label').textContent = 'Currently: Dark';
  }
  // Pre-render QR grid (lazy)
  // Render navigate on screen open
  goTo('login');
});
