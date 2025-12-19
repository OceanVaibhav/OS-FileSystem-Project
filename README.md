# 📂 Virtual File System & Recovery Tool (OS Project)

![Project Status](https://img.shields.io/badge/Status-Completed-success)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20%7C%20Python-blue)
![Frontend](https://img.shields.io/badge/Frontend-HTML%2FCSS%2FJS-orange)

A simulation of a standard Operating System File Management component. This project implements a **Virtual Disk**, **Inode-based File System**, and advanced features like **Crash Recovery** and **Disk Defragmentation**.

The system uses a **C++ Engine** for low-level disk operations and a **Python Flask + Web Dashboard** for a modern, full-stack user interface.

## 🚀 Features

### Core File System
* **Create Files:** Allocates blocks on the virtual disk and updates the inode table.
* **Read Files:** Retrieves data directly from specific block indices.
* **Update Files:** Modifies content and handles size adjustments.
* **Delete Files:** Marks inodes as free and updates the superblock.
* **Persistence:** All data is saved to `vdisk.dat`, so files remain after closing the program.

### Advanced OS Concepts
* **🔥 Crash Simulation & Recovery:**
    * Simulates a sudden power failure by setting a "Dirty Bit" in the Superblock.
    * On restart, the system detects the unclean shutdown and runs an automated `fsck` (File System Check) to repair inconsistencies.
* **⚡ Disk Defragmentation:**
    * Scans the disk for fragmentation (scattered blocks).
    * Reorganizes file blocks contiguously to optimize read speeds and reclaim space.

---

## 🛠️ Technology Stack

* **Backend Engine:** C++ (Handles memory, pointers, and binary file I/O).
* **Middleware:** Python (Flask) acts as a bridge between the C++ engine and the browser.
* **Frontend:** HTML5, CSS3 (Glassmorphism UI), JavaScript (Async API calls).
* **Storage:** `vdisk.dat` (A binary file acting as the physical hard drive).

---

## ⚙️ Installation & Run Instructions

### Prerequisites
* G++ Compiler (MinGW for Windows)
* Python 3.x

### Step 1: Compile the Engine
Open your terminal and compile the C++ backend:
```bash
g++ main.cpp -o myfs

### Step 2: Compile the Engine
python server.py

📝 Project Structure

/OS-FileSystem-Project
│── main.cpp           # The Brain (C++ File System Logic)
│── myfs.exe           # Compiled Executable
│── server.py          # Python Flask Server
│── vdisk.dat          # Virtual Hard Disk File
│── templates/
│   └── index.html     # Frontend UI Structure
└── static/
    ├── style.css      # Dark Mode Styling
    └── script.js      # API Fetch Logic

👨‍💻 Contributors
• Vaibhav (12419781)
• Sudhanshu (12420005)
• Harsh (12419685)