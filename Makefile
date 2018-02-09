CXX = g++ 
CXXFLAGS = -g -Wall -std=c++11 -pthread -O3 -flto -DNDEBUG

SOURCES = $(wildcard ./*.cpp ./*.c ./minizip/*.c)
HEADERS = 
INCLUDE = -I/usr/include/libxml2/

LDFLAGS = -L
LIBS = -lz -lxml2

LIBNAME = libmyutility.so
LIBOBJ =  File.o log.o xmlConf.o ZipSerialize.o
LIBOBJ_C = ./minizip/ioapi.o ./minizip/iowin32.o ./minizip/unzip.o ./minizip/zip.o

$(LIBNAME): $(LIBOBJ) $(LIBOBJ_C)
	$(CXX) $(CXXFLAGS) -fPIC -shared $(INCLUDE) $(LDFLAGS) $(LIBS) -o $@ $^
	
$(LIBOBJ): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC $(INCLUDE) $(LDFLAGS) $(LIBS) -c -o $@ $^

$(LIBOBJ_C): %.o: %.c
	$(CXX) $(CXXFLAGS) -fPIC $(INCLUDE) $(LDFLAGS) $(LIBS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f $(LIBOBJ) $(LIBOBJ_C) $(LIBNAME)
