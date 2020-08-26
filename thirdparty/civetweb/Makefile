TARGET_DIR = ../../build/

all:
	cd civetweb-1.11 && make clean lib WITH_CPP=1 WITH_ZLIB=1 WITH_IPV6=1 WITH_WEBSOCKET=1 WITH_SERVER_STATS=1 \
	COPT='-DOPENSSL_API_1_1 -DMAX_PARAM_BODY_LENGTH=16*1024*1024'

clean:
	cd civetweb-1.11 && make clean

install:
	install ./civetweb-1.11/libcivetweb* $(TARGET_DIR)/lib
	install -d $(TARGET_DIR)/include/civetweb 
	install ./civetweb-1.11/include/* $(TARGET_DIR)/include/civetweb
