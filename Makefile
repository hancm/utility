CXX = g++ 
CXXFLAGS = -g -Wall -std=c++11 -pthread -O3 -flto -DNDEBUG

SOURCES = $(wildcard ./*.cpp ./minizip/*.cpp)
HEADERS = 
INCLUDE = -I/usr/include/libxml2/

LDFLAGS = -L
LIBS = -lz -lxml2

LIBNAME = libmyutility.so
LIBOBJ = ./minizip/ioapi.o ./minizip/iowin32.o ./minizip/unzip.o ./minizip/zip.o File.o log.o xmlConf.o ZipSerialize.o

$(LIBNAME): $(LIBOBJ)
	$(CXX) $(CXXFLAGS) -fPIC -shared $(INCLUDE) $(LDFLAGS) $(LIBS) -o $@ $^
	
$(LIBOBJ): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC $(INCLUDE) $(LDFLAGS) $(LIBS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f $(LIBOBJ) $(LIBOBJ_C) $(LIBNAME)
