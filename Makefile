# Simple Makefile for building server and client

CXX = g++
CXXFLAGS = -Wall -std=c++11

all: server client

server: server.cpp bank.h
	$(CXX) $(CXXFLAGS) server.cpp -o server

client: client.cpp bank.h
	$(CXX) $(CXXFLAGS) client.cpp -o client

clean:
	rm -f server client
