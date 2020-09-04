BUILD_PATH := $(PWD)/build/
TARGET_DIR := $(PWD)/../../build/

TAR_LIB := tacopie-3.2.0.tar.gz
LIB_PATH := tacopie-3.2.0
INSTALL_PREFIX := $(BUILD_PATH)

all:
	mkdir -p $(INSTALL_PREFIX) && cd $(INSTALL_PREFIX) && cmake -DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) ../$(LIB_PATH) && make && make install

clean:
	rm -rf $(INSTALL_PREFIX)

install:
	install ./build/lib/* $(TARGET_DIR)/lib
	cp -rf $(BUILD_PATH)/include/* $(TARGET_DIR)/include/
