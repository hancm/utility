SHELL = /bin/bash

TARGET = http

CXX = g++
CXXFLAGS = -std=c++11 -g -Wall -O2 -pthread -flto #-DNDEBUG
INCLUDE = -I./ -I./utility \
		  -I../utility \
		  -I../build/ -I../build/include/ -I../build/include/civetweb
LDFLAGS = -L./ -Wl,-rpath=./ \
		  -L./lib -Wl,-rpath=./lib \
		  -L../build -Wl,-rpath=../build/ \
		  -L../build/lib/ -Wl,-rpath=../build/lib/
LIBS = -ldl -lz -lcivetweb 

SOURCES = $(wildcard ./*.cpp) ../utility/pugixml/pugixml.cpp ../utility/pugixml_utility.cpp 
OBJS = $(SOURCES:.cpp=.o)
DEPS = $(SOURCES:.cpp=.d)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

BUILD_DIR = ../build/
install:
	install -d $(BUILD_DIR)
	install -d $(BUILD_DIR)/include
	install $(TARGET) $(BUILD_DIR)
	install -d $(BUILD_DIR)/conf
	install ./conf/http-config.xml $(BUILD_DIR)/conf
	install server.sh $(BUILD_DIR)
	
ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MF $*.d -MP -MT $@ -c -o $@ $<

%.o: %.c
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MF $*.d -MP -MT $@ -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS) $(DEPS) *.o *.d
