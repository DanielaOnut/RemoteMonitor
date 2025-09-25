# Remote Monitor

Remote Monitor is a task manager app written in C++ using Qt that gathers multiple machines into one client app. 

## Features
- Real-time CPU, RAM, and disk usage
- Process list (sortable by name, CPU, or RAM usage)
- System information (e.g., OS architecture)
- Kill processes remotely
- Multi-machine support

<img width="912" height="487" alt="ss1" src="https://github.com/user-attachments/assets/94a6a1d9-1006-4d32-ade4-8f9a6c270c66" />
<img width="912" height="347" alt="ss2" src="https://github.com/user-attachments/assets/d0defa65-702b-4582-8924-cf4cda30c867" />

## Usage  
Run `./server` executable on each targeted machine. The server works as a daemon.

Run `./client` on the client machine.
