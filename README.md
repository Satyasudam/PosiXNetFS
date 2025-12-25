# DCFS: Distributed Collaborative Filesystem (PosiXNetFS)

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Platform](https://img.shields.io/badge/platform-Linux-blue)
![License](https://img.shields.io/badge/license-MIT-green)

**DCFS** is a lightweight, distributed filesystem implementation for Linux that allows multiple clients to mount a shared virtual directory. It features an "offline-first" architecture with aggressive local caching, allowing users to work at local disk speeds while a background daemon handles synchronization, conflict detection, and versioning.

This project demonstrates advanced Linux System Programming concepts including **FUSE (Filesystem in Userspace)**, **Inter-Process Communication (Unix Domain Sockets)**, **Multi-threaded Network Programming**, and **Optimistic Concurrency Control**.

---

##  Key Features

* **Local-Speed Performance:** Uses a local cache for all reads and writes; network operations are asynchronous.
* **Offline Capability:** Continue editing files even when the server is unreachable. Changes sync automatically upon reconnection.
* **Conflict Detection:** Server-side versioning prevents data loss. Conflicting edits are preserved and renamed (e.g., `file.txt.conflict`).
* **Split-Process Architecture:** Decouples the filesystem interface (FUSE) from networking logic (Sync Daemon) for maximum responsiveness.
* **Efficient Metadata:** Uses SQLite for robust management of file versions, permissions, and locks.

---

##  Architecture

The system consists of three main components:

1.  **DCFS Client (FUSE):** Intercepts kernel filesystem calls. It reads/writes directly to a hidden local cache directory (`~/.dcfs_cache`) and notifies the daemon via IPC.
2.  **Sync Daemon:** A background process that listens for IPC messages from the FUSE client. It manages the TCP connection to the server, uploading "dirty" files and polling for remote updates.
3.  **DCFS Server:** A multi-threaded TCP server using `epoll` for high concurrency. It persists files to disk and manages metadata in a SQLite database.

   
 Prerequisites

To build DCFS, you need a Linux environment with the following dependencies:

    GCC (or Clang)

    CMake (3.10+)

    FUSE 3 (libfuse3-dev)

    SQLite 3 (libsqlite3-dev)

Install on Ubuntu/Debian:
Bash

sudo apt-get update
sudo apt-get install build-essential cmake libfuse3-dev libsqlite3-dev

Install on Fedora/CentOS:
Bash

sudo dnf install cmake gcc-c++ fuse3-devel sqlite-devel

 Build Instructions

    Clone the repository:
    Bash

git clone [https://github.com/yourusername/dcfs.git](https://github.com/yourusername/dcfs.git)
cd dcfs

Create a build directory and compile:
Bash

    mkdir build
    cd build
    cmake ..
    make

This will generate the following executables:

    ./dcfs_server

    ./dcfs_client

    ./dcfs_sync_daemon

    ./dcfs_cli

 Usage Guide

To run the system, you will need three separate terminal windows.
Step 1: Start the Server (Terminal 1)

Run the server. It will create a storage_root directory in the current folder.
Bash
```bash
./dcfs_server
```
# Output: Server listening on port 8080...

Step 2: Start the Sync Daemon (Terminal 2)

Run the background daemon. This handles networking for the client.
Bash
```bash
./dcfs_sync_daemon
```
# Output: Daemon started. Listening for IPC on /tmp/dcfs.sock...

Step 3: Mount the Filesystem (Terminal 3)

Create a mount point and start the FUSE client.
Bash
```bash
mkdir mnt
./dcfs_client mnt
```
# Output: Filesystem mounted at mnt/

Step 4: Interact!

You can now use the mnt directory like a normal folder.
Bash
```bash
cd mnt
echo "Hello Distributed World" > hello.txt
ls -l
cat hello.txt
```
Check the output in Terminal 2 (Daemon) and Terminal 1 (Server) to see the file being uploaded in the background!
