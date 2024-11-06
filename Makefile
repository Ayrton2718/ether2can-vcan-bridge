CC = g++
CFLAGS = -Wall -Wextra -g $(shell pkg-config --cflags libnl-3.0 libnl-route-3.0)
LIBS = $(shell pkg-config --libs libnl-3.0 libnl-route-3.0)

TARGET = ether2can-vcan-bridge
SRC = $(wildcard src/*.c src/*.cpp)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
