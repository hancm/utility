SOCI_LIB_DIR=./soci
PCAPPLUSPLUS_LIB_DIR=./PcapPlusPlus

all: build_dir third

third:
	cd ${SOCI_LIB_DIR} && make && make install
	cd ${PCAPPLUSPLUS_LIB_DIR} && make && make install

clean:
	cd ${SOCI_LIB_DIR} && make clean
	cd ${PCAPPLUSPLUS_LIB_DIR} && make clean

build_dir:
	mkdir -p ../build/include
	mkdir -p ../build/lib
	mkdir -p ../build/config

.PHONY: all third clean build_dir
