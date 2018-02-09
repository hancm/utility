SOURCES = $(wildcard ./*.cpp ./*.c ./minizip/*.c)
LIBS = -lz
all:
	g++ -g -std=c++11 -W -pthread -O3 -flto -DNDEBUG -o test $(SOURCES) $(LIBS)
