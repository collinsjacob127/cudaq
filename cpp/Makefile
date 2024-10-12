# Makefile for Shor's Alg
EXE = shors.x
FILENAME = shors.cpp
CFLAGS = -Wall -g
CXXFLAGS = -Wall
LDLIBS = 
CC = nvcc
CXX = nvq++

.PHONY: all
all: $(EXE)

$(EXE): $(FILENAME)
	$(CXX) $(CXXFLAGS) $(FILENAME) $(LDLIBS) -o $(EXE)

.PHONY: clean
clean:
	rm -f $(EXE)
