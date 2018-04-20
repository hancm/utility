TARGET  := test
  
#compile and lib parameter
#编译参数
CC      := g++
3RD		:= 
LIBS    := -lmupdf -lmupdfthird  -lpthread #-Wl,-rpath-link ../../src  -Wl,-rpath-link $(3RD)  -Wl,-rpath ../../src -Wl,-rpath $(3RD) -lpdfhelper -lgtest -lgtest_main 
LDFLAGS :=
DEFINES :=

INCLUDE :=
CFLAGS := -O3 -pthread -flto -DNDEBUG -m64 -fPIC -Wall -std=c++11  $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H

#i think you should do anything here
#下面的基本上不需要做任何改动了
  
#source file
#源文件，自动找所有.c和.cpp文件，并将目标定义为同名.o文件
SOURCE  := $(wildcard *.c) $(wildcard *.cpp)
OBJS    := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
  
.PHONY : everything objs clean veryclean rebuild
  
everything : $(TARGET)
  
all : $(TARGET)
  
objs : $(OBJS)
  
rebuild: veryclean everything
                
clean :
	rm -fr *.o test
    
veryclean : clean
	rm -fr $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)
