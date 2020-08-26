INSTALL_PATH := $(PWD)/build/
TARGET_PATH := $(PWD)/../../build/

TAR_LIB := PcapPlusPlus-v19.12.zip
LIB_PATH := $(PWD)/PcapPlusPlus

all:
	install -d $(INSTALL_PATH)
	unzip $(TAR_LIB) && chmod -R +x $(LIB_PATH)
	cd $(LIB_PATH) && ./configure-linux.sh --use-immediate-mode --set-direction-enabled --install-dir $(INSTALL_PATH) && make && make install

install:
	cd $(INSTALL_PATH) && cp -rf ./include/* $(TARGET_PATH)/include && cp -rf ./lib/*.a $(TARGET_PATH)/lib/
	
clean:
	rm -rf $(INSTALL_PATH) 
	rm -rf $(LIB_PATH)
	rm -rf $(TARGET_PATH)/include/pcapplusplus $(TARGET_PATH)/lib/libPcap++.a $(TARGET_PATH)/lib/libPacket++.a $(TARGET_PATH)/lib/libCommon++.a
	
.PHONY: all install clean
