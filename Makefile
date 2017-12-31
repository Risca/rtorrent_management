.PHONY: all install clean
all: rtm

rtm: main.cpp
	g++ $^ -std=c++11 $(shell xmlrpc-c-config c++2 client --cflags --libs) -o $@

install: rtm
	install -m 0755 rtm /usr/local/bin/

clean:
	rm -f rtm
