CC      = gcc
CFLAGS  = -Wall -Wextra -g -Iinclude
LDFLAGS = -lpthread

SRC    = $(wildcard src/*.c)
OBJ    = $(patsubst src/%.c, build/%.o, $(SRC))
TARGET = simulador

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build $(TARGET)

.PHONY: all clean
