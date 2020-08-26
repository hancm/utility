TARGET_PATH := $(PWD)/../build/

TAR_LIB := openssl-1.1.1b.tar.gz
LIB_PATH := openssl-1.1.1b
INSTALL_PREFIX := $(PWD)/build/

all:
	tar -zxf $(TAR_LIB) && chmod -R +x $(LIB_PATH)
	mkdir -p $(INSTALL_PREFIX) && cd $(LIB_PATH) && ./config --prefix=$(INSTALL_PREFIX) && make && make install

install:
	install -d $(TARGET_PATH)/include/openssl
	cp -rf $(INSTALL_PREFIX)/include/openssl/* $(TARGET_PATH)/include/openssl
	cp -rf $(INSTALL_PREFIX)/lib/libcrypto.a $(TARGET_PATH)

clean:
	rm -rf $(LIB_PATH) && rm -rf $(INSTALL_PREFIX)

.PHONY: all clean
