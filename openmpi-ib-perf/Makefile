CXX=mpic++
#CXX?=g++
CXXFLAGS?=-fPIC -O3 -g -Wall -Wno-unused-function -std=c++11  -finline-functions
#LDFLAGS?=-L/home/zork/loclib/mpich-3.1.4/lib -lpthread -lglog -lmpich -lgflags -lboost_serialization -llapack -lrdmacm -libverbs
#INCLUDES=-I. -I/home/zork/loclib/mpich-3.1.4/include -I/usr/local/include/boost-numeric-bindings

LDFLAGS?=-L/usr/lib/openmpi/lib -lpthread -lmpi -lglog  -lgflags
INCLUDES=-I. -I/usr/include/openmpi

all : main

main : main.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)


main.o : main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

.PHONY:
clean : 
	rm -f main
	rm -f main.o
