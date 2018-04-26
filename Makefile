SHELL = /usr/bin/bash

CXX = g++
CXXFLAGS = -std=c++11 -Wall -O3 -pthread -flto -DNDEBUG -m64 -fPIC

TARGET = myutility_test

SOURCES = $(wildcard ./*.cpp) $(wildcard ./minizip/*.cpp)

INCLUDE = -I/usr/include/libxml2/
LDFLAGS =
LIBS = -lz -lxml2

OBJS = $(SOURCES:.cpp=.o)
DEPS = $(SOURCES:.cpp=.d)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

lib: $(OBJS)
	ar rcs $@ $^

#test: $(TARGET) test_main.o
#	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MF $*.d -MP -MT $@ -c -o $@ $<
	
%.o: %.c
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MF $*.d -MP -MT $@ -c -o $@ $<
	
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS) $(DEPS) *.o *.d test
