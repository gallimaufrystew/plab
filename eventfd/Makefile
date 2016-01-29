CXX?=g++
CXXFLAGS?=-std=c++11 -O3 -g -Wall
LDFLAGS?=-lpthread -lglog 
INCLUDES?=-I.

all : main

main : main.o
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

main.o : main.cpp 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@


.PHONY:
clean : 
	rm -f *.o
	rm -f main
