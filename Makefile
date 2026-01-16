CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pedantic

TARGET := log_parser
SRC := src/main.cpp

.PHONY: help all build run clean

help:
	@echo "make build   - compile the program"
	@echo "make run     - run using sample_logs/sample.log"
	@echo "make clean   - remove the compiled binary"

all: build

build:
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: build
	./$(TARGET) sample_logs/sample.log

clean:
	rm -f $(TARGET)

