CXXFLAGS := -std=c++11
ifneq ($(ERASE_FOR_REAL),)
	CXXFLAGS += -DERASE_FOR_REAL
endif

.PHONY: all install clean
all: rtm

rtm: main.cpp
	g++ $^ $(CXXFLAGS) $(shell xmlrpc-c-config c++2 client --cflags --libs) -o $@

install: rtm
	install -m 0755 rtm /usr/local/bin/

clean:
	rm -f rtm
