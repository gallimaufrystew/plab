CXX?=g++
CXXFLAGS?=-std=c++11 -O3 -g -Wall
LDFLAGS?=-lpthread -lglog 
INCLUDES?=-I.

all : main test

main : main.o
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

main.o : main.cpp 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

test : test.o
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

test.o : test.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@



.PHONY:
clean : 
	rm -f *.o
	rm -f main
	rm -f test
