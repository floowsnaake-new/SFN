# Makefile for OpenDoor.c

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2

# Source file and target executable
SRCS = OpenDoor.c
TARGET = OpenDoor

# Default build command
all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Clean command
clean:
	rm -f $(TARGET)

.PHONY: all clean
