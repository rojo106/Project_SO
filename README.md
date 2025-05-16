This project is a UNIX system programming assignment that simulates a treasure hunt game system using:

- File handling
- Process management
- Signals
- Inter-process communication (pipes)
- External program integration

### Project Structure


├── src/                    

│   ├── treasure_manager.c

│   ├── treasure_hub.c

│   ├── monitor.c

│   └── score_calculator.c

├── bin/                    # Binaries compilés

├── hunts/                  # Répertoires générés pour chaque chasse

├── Makefile                # Instructions de compilation

├── monitor_command         # Fichier de commande pour communication par signaux

├── logged_hunt-<ID>        # Liens symboliques vers fichiers log




### Features by Phase

- **Phase 1: File System (`treasure_manager`)**  
  Add, list, view, and remove treasures; create and delete hunts; log all actions.

- **Phase 2: Processes and Signals (`treasure_hub`, `monitor`)**  
  Background monitor process; command signaling; hunt and treasure listings.

- **Phase 3: Pipes and Score Calculation**  
  Monitor output redirected via pipe; external score calculator program; user scores.

### How to Compile

Make sure you have `gcc` and `make` installed:

bash :

make        ||  # Compile everything

make run    ||  # Launch treasure_hub

make clean  ||  # Clean binaries and temp files


How to Use

treasure_manager commands:

./bin/treasure_manager --add Hunt001

./bin/treasure_manager --list Hunt001

./bin/treasure_manager --view Hunt001 1

./bin/treasure_manager --remove_treasure Hunt001 1

./bin/treasure_manager --remove Hunt001


Inside treasure_hub interactive shell:

start_monitor

list_hunts

list_treasures Hunt001

calculate_score

stop_monitor

exit




