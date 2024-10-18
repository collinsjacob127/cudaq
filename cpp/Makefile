# Makefile for CudaQ
CFLAGS = -Wall -g
CXXFLAGS = -Wall
LDLIBS = 
CC = gcc
CXX = nvq++

default: shors.o

# make <filename>.o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@
    
# make <filename> == make <filename>.o
%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@.o

.PHONY: default clean

clean:
	rm -f *.o
