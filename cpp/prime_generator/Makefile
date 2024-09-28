# Authors: Jacob Collins
# Assignment 2 Makefile
# CSCI 411
# Spring 2024
EXE = a.out
FILENAME = generate_primes.cpp
CFLAGS = -Wall -g
CXXFLAGS = -Wall
LDLIBS = bignum.cpp
CC = gcc
CXX = g++

.PHONY: all
all: $(EXE)

$(EXE): $(FILENAME)
	$(CXX) $(CXXFLAGS) $(FILENAME) $(LDLIBS) -o $(EXE)

.PHONY: clean
clean:
	rm -f $(EXE)
