.PHONY: all
all: rtm

rtm: main.cpp
	g++ $^ -std=c++11 $(shell xmlrpc-c-config c++2 client --cflags --libs) -o $@
