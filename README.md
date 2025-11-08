Modular TCP Chat System (C)

A real-time multi-client chat application built in pure C, featuring a modular event-driven architecture and cross-platform communication between a Linux server and a Windows client.

🧠 Overview

Server (Linux): TCP sockets with select() event loop

Handles concurrent clients without threading

Supports /login, /who, /join, /msg, /quit commands

Includes heartbeat timeout detection and structured logging

Client (Windows): Winsock-based multithreaded terminal

Colorized UI with real-time input/output

Connects to the server and supports all chat commands

🏗️ Project Structure
chat-system-c/
├── client/    # Windows client (Winsock, multithreaded UI)
└── server/    # Linux server (select(), registry, router, log, etc.)

⚙️ Build Instructions
🖥️ Server (Linux)
cd server
mkdir build && cd build
cmake .. && cmake --build .
./chat_server <port>


💬 Client (Windows)
cd client
mkdir build; cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
.\chat_client.exe <host> <port>

🧩 Features

Event-driven TCP communication

Multi-room chat support

Private messages (/msg <user> <text>)

Active user list (/who)

Room switching (/join <room>)

Connection timeout handling

Centralized structured logging

🚀 Tech Stack

C (C11)

POSIX sockets (server)

Winsock2 (client)

CMake for build configuration

🧑‍💻 Author

Developed by Robert Postu — educational systems programming project demonstrating low-level networking, concurrency, and clean C architecture.
