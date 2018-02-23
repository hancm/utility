SHELL = /usr/bin/bash

CXX = g++ -std=c++11
CXXFLAGS = -g -Wall -pthread -O3 -flto -DNDEBUG

TARGET = myutility_test

SOURCES = $(wildcard ./*.cpp) $(wildcard ./minizip/*.cpp)

INCLUDE = -I/usr/include/libxml2/
LDFLAGS = -L
LIBS = -lz -lxml2

OBJS = $(SOURCES:.cpp=.o)
DEPS = $(SOURCES:.cpp=.d)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LDFLAGS) $(LIBS) -o $@ $^
	
ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LDFLAGS) $(LIBS) -o $@ -c $< -MMD -MF $*.d -MP
	
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS) $(DEPS)
