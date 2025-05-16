CC = gcc
CFLAGS = -Wall -Wextra
BIN_DIR = bin

all: $(BIN_DIR)/treasure_manager $(BIN_DIR)/treasure_hub $(BIN_DIR)/monitor $(BIN_DIR)/score_calculator

$(BIN_DIR)/treasure_manager: treasure_manager.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/treasure_hub: treasure_hub.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/monitor: monitor.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/score_calculator: score_calculator.c
	$(CC) $(CFLAGS) $< -o $@

run:
	./$(BIN_DIR)/treasure_hub

clean:
	rm -f $(BIN_DIR)/*
	rm -f monitor_command
	rm -f logged_hunt-*
