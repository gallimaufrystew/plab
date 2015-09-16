
export LD_LIBRARY_PATH=/home/zork/loclib/papi-5.4.1/lib

CXX?=g++
CXXFLAGS=-O3 -g -std=c++11
LDFLAGS=-L/home/zork/loclib/papi-5.4.1/lib/ -lgflags -lglog -lpapi
INCLUDES=-I. -I/home/zork/loclib/papi-5.4.1/include/

all : main

main : main.o
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

main.o : main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

.PHONY:
clean :
	rm -f main.o
	rm -f main
