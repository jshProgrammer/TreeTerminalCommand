CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pthread
TARGET = tree
SOURCES = main.c modules/logic.c modules/tree.c modules/queue.c modules/tests.c modules/globals.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(TARGET)

test: $(TARGET)
	./$(TARGET) --test

.PHONY: all clean test
