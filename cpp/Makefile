# Makefile for Shor's Alg
EXE = a.out
FILENAME = shors.cpp
CFLAGS = -Wall -g
CXXFLAGS = -Wall
LDLIBS = 
CC = gcc
CXX = g++

.PHONY: all
all: $(EXE)

$(EXE): $(FILENAME)
	$(CXX) $(CXXFLAGS) $(FILENAME) $(LDLIBS) -o $(EXE)

.PHONY: clean
clean:
	rm -f $(EXE)
