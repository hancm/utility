CXX = g++ -std=c++11
CXXFLAGS = -g -Wall -pthread -O3 -flto -DNDEBUG

TARGET = myutility_test

SOURCES = $(wildcard ./*.cpp) $(wildcard ./minizip/*.cpp)
HEADERS = 

INCLUDE = -I/usr/include/libxml2/
LDFLAGS = -L
LIBS = -lz -lxml2

OBJS = $(SOURCES:.cpp=.o)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LDFLAGS) $(LIBS) -o $@ $^
	
$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LDFLAGS) $(LIBS) -c -o $@ $^
	
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS) 
