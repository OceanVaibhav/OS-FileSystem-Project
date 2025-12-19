const API = "/api";
const TOTAL_BLOCKS = 50;
const RESERVED = 5;
let currentEditingFile = ""; // Track what we are editing

document.addEventListener('DOMContentLoaded', () => {
    fetchData();
    log("System Dashboard Loaded.");
});

async function fetchData() {
    const res = await fetch(`${API}/list`);
    const data = await res.json();
    
    if (data.message) {
        showToast("⚠️ RECOVERY: " + data.message, "error");
        document.getElementById('health-indicator').classList.add('error');
        document.getElementById('status-text').innerText = "RECOVERED";
        log(`[KERNEL] CRITICAL: ${data.message}`);
    } else {
        document.getElementById('health-indicator').classList.remove('error');
        document.getElementById('status-text').innerText = "ONLINE";
    }

    renderGrid(data.files);
    renderTable(data.files);
}

function renderGrid(files) {
    const grid = document.getElementById('diskGrid');
    grid.innerHTML = "";
    let usedBlocks = new Set();
    files.forEach(f => usedBlocks.add(f.block));

    for (let i = 0; i < TOTAL_BLOCKS; i++) {
        let div = document.createElement('div');
        div.className = "block";
        if (i < RESERVED) {
            div.classList.add("sys");
            div.title = `Block ${i}: SYSTEM RESERVED`;
        } else if (usedBlocks.has(i)) {
            div.classList.add("used");
            div.title = `Block ${i}: USED`;
        } else {
            div.classList.add("free");
            div.title = `Block ${i}: Free`;
        }
        grid.appendChild(div);
    }
}

function renderTable(files) {
    const tbody = document.getElementById('fileTable');
    tbody.innerHTML = "";
    
    if (files.length === 0) {
        tbody.innerHTML = `<tr><td colspan="4" style="text-align:center; color:#64748b;">Disk is empty</td></tr>`;
        return;
    }

    files.forEach(f => {
        let tr = `<tr>
            <td>📄 ${f.name}</td>
            <td><span style="background:#334155; padding:2px 6px; border-radius:4px;">BLK ${f.block}</span></td>
            <td>${f.size} B</td>
            <td>
                <span onclick="readFile('${f.name}')" class="action-link">View</span>
                <span onclick="openEditModal('${f.name}')" class="action-link" style="color:#facc15;">Edit</span>
                <span onclick="deleteFile('${f.name}')" class="action-link del">Delete</span>
            </td>
        </tr>`;
        tbody.innerHTML += tr;
    });
}

// === FIX: Function to Open Create Modal AND Clear Inputs ===
function openCreateModal() {
    document.getElementById('fileName').value = ""; // Clear name
    document.getElementById('fileContent').value = ""; // Clear content
    document.getElementById('createModal').style.display = 'flex';
}

async function createFile() {
    const name = document.getElementById('fileName').value;
    const content = document.getElementById('fileContent').value;
    if(!name) return showToast("Filename required", "error");

    log(`[USER] Requesting creation of '${name}'...`);
    const res = await fetch(`${API}/create`, {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({name, content})
    });
    const data = await res.json();
    
    closeModal('createModal');
    if(data.status.includes("SUCCESS")) {
        showToast("File Created Successfully");
        log(`[KERNEL] SUCCESS: Wrote '${name}' to disk.`);
    } else {
        showToast(data.status, "error");
        log(`[KERNEL] ERROR: ${data.status}`);
    }
    fetchData();
}

async function readFile(name) {
    log(`[USER] Reading '${name}'...`);
    const res = await fetch(`${API}/read`, {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({name})
    });
    const data = await res.json();
    document.getElementById('readTitle').innerText = name;
    document.getElementById('readBody').innerText = data.content;
    document.getElementById('readModal').style.display = 'flex';
}

// === NEW: Edit Functionality ===
async function openEditModal(name) {
    currentEditingFile = name;
    document.getElementById('editFileNameDisplay').innerText = name;
    
    // Fetch current content first
    const res = await fetch(`${API}/read`, {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({name})
    });
    const data = await res.json();
    
    document.getElementById('editFileContent').value = data.content;
    document.getElementById('editModal').style.display = 'flex';
}

async function saveEdit() {
    const content = document.getElementById('editFileContent').value;
    
    log(`[USER] Updating '${currentEditingFile}'...`);
    const res = await fetch(`${API}/update`, {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({name: currentEditingFile, content})
    });
    const data = await res.json();
    
    closeModal('editModal');
    if(data.status.includes("SUCCESS")) {
        showToast("File Updated Successfully");
        log(`[KERNEL] SUCCESS: Updated content for '${currentEditingFile}'.`);
    } else {
        showToast(data.status, "error");
        log(`[KERNEL] ERROR: ${data.status}`);
    }
    fetchData();
}

async function deleteFile(name) {
    if(!confirm("Delete " + name + "?")) return;
    log(`[USER] Deleting '${name}'...`);
    await fetch(`${API}/delete`, {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({name})
    });
    showToast("File Deleted");
    log(`[KERNEL] Block freed for '${name}'.`);
    fetchData();
}

async function optimizeDisk() {
    log(`[SYSTEM] Starting Defragmentation Routine...`);
    showToast("Optimization Started...", "info");
    const res = await fetch(`${API}/optimize`, { method: 'POST' });
    const data = await res.json();
    
    setTimeout(() => {
        log(`[KERNEL] ${data.status}`);
        showToast("Optimization Complete");
        fetchData();
    }, 1000); 
}

async function simulateCrash() {
    log(`[SYSTEM] SIMULATING POWER FAILURE...`);
    await fetch(`${API}/crash`, { method: 'POST' });
    alert("SYSTEM HALTED. Refresh page to trigger recovery.");
}

function log(msg) {
    const term = document.getElementById('sysLog');
    const time = new Date().toLocaleTimeString();
    term.innerHTML += `<div class="log-line"><span style="color:#64748b">[${time}]</span> ${msg}</div>`;
    term.scrollTop = term.scrollHeight;
}

function showToast(msg, type="success") {
    const container = document.getElementById('toast-container');
    const div = document.createElement('div');
    div.className = "toast";
    div.innerText = msg;
    if(type === "error") div.style.borderLeftColor = "#ef4444";
    container.appendChild(div);
    setTimeout(() => div.remove(), 3000);
}

function closeModal(id) { document.getElementById(id).style.display = 'none'; }