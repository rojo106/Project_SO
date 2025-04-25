# Treasure Hunt System — Phase: Monitor Interaction

## 📄 Description

This project implements a signal-based multi-process interaction system for a treasure hunt game. The main controller `treasure_hub` communicates with a background process (monitor mode of `treasure_manager`) using signals and a shared command file.

## 🧠 Key Features

- 🧵 **Process creation** via `fork()` and `exec()`
- 📡 **Signal communication** between `treasure_hub` and the monitor:
  - `SIGUSR1`: New command available
  - `SIGUSR2`: Request monitor termination
- 📁 **Command file (`cmd.txt`)** used to pass instructions
- 🪦 **Signal handler for SIGCHLD** to detect monitor termination
- 💤 Simulated delay on monitor exit (`usleep()`)

## 🧪 Commands (via `treasure_hub`)

| Command                  | Description                                   |
|--------------------------|-----------------------------------------------|
| `start_monitor`          | Launches the monitor in the background        |
| `list_hunts`             | Lists all active hunts and their treasure counts |
| `list_treasures <hunt>`  | Lists all treasures of a given hunt           |
| `view_treasure <hunt> <id>` | Displays a specific treasure               |
| `stop_monitor`           | Terminates the monitor process cleanly        |
| `exit`                   | Exits `treasure_hub` (only if monitor is stopped) |

## 🚀 How to Build and Run

```bash
gcc treasure_manager.c -o treasure_manager
gcc treasure_hub.c -o treasure_hub
./treasure_hub
