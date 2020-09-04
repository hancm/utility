BUILD_PATH := $(PWD)/build/
TARGET_DIR := $(PWD)/../../build/

TAR_LIB := cpp_redis-4.3.1.tar.gz
LIB_PATH := cpp_redis-4.3.1
INSTALL_PREFIX := $(BUILD_PATH)

all:
	mkdir -p $(INSTALL_PREFIX) && cd $(INSTALL_PREFIX) && cmake -DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) -DLOGGING_ENABLED=1 -DTACOPIE_LIBRARY=$(TARGET_DIR)/lib/libtacopie.a -DTACOPIE_INCLUDE_DIR=$(TARGET_DIR)/include ../$(LIB_PATH) && make && make install

clean:
	rm -rf $(INSTALL_PREFIX)

install:
	install ./build/lib/lib* $(TARGET_DIR)/lib
	cp -rf $(BUILD_PATH)/include/* $(TARGET_DIR)/include/
