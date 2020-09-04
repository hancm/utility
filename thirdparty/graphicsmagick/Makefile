TARGET_PATH := $(PWD)/../../build/

TAR_LIB := GraphicsMagick-1.3.33.tar.gz
LIB_PATH := GraphicsMagick-1.3.33
INSTALL_PREFIX := $(PWD)/build/

all:
	tar -zxf $(TAR_LIB) && chmod -R +x $(LIB_PATH)
	mkdir -p $(INSTALL_PREFIX) && cd $(LIB_PATH) && ./configure --prefix=$(INSTALL_PREFIX) -enable-shared && make && make install
	cp -rf $(INSTALL_PREFIX)/include/* $(TARGET_PATH)/include/ && cp -rf $(INSTALL_PREFIX)/lib/*.so* $(TARGET_PATH)/lib

clean:
	rm -rf $(LIB_PATH) && rm -rf $(INSTALL_PREFIX) $(TARGET_PATH)/include/GraphicsMagick $(TARGET_PATH)/lib/libGraphicsMagick*.so*

.PHONY: all clean
