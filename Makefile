# Makefile for Polynomial Language Parser
# Author: Generated for educational purposes

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
TARGET = poly_parser

SRCS = inputbuf.cc lexer.cc parser.cc
HDRS = inputbuf.h lexer.h parser.h
OBJS = $(SRCS:.cc=.o)

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cc $(HDRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

test: $(TARGET)
	./run_tests.sh

# Run a single test file
# Usage: make run INPUT=tests/test_basic.txt
run: $(TARGET)
	./$(TARGET) < $(INPUT)
