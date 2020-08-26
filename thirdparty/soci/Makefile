INSTALL_PATH := $(PWD)/build/
TARGET_PATH := $(PWD)/../../build/

LIB_PATH := $(PWD)/soci-3.2.3/src
CONDA_DIR := /opt/anaconda3/envs/capture/

all:
	mkdir -p $(INSTALL_PATH) && cd $(INSTALL_PATH) && cmake -G "Unix Makefiles" -DSOCI_DB2=OFF -DSOCI_ODBC=OFF -DSOCI_ORACLE=OFF -DSOCI_FIREBIRD=OFF -DSOCI_POSTGRESQL=OFF -DSOCI_TESTS=OFF -DSOCI_EMPTY=OFF -DSOCI_SQLITE3=OFF -DWITH_BOOST=OFF -DSOCI_USE_BOOST=OFF -DSOCI_CXX_C11=ON -DWITH_MYSQL=ON -DSOCI_MYSQL=ON -DMYSQL_INCLUDE_DIR=$(CONDA_DIR)/include -DMYSQL_LIBRARIES=$(CONDA_DIR)/lib -DCMAKE_INSTALL_PREFIX=$(INSTALL_PATH) $(LIB_PATH) && make && make install

install:
	cp -rf $(INSTALL_PATH)/include/* $(TARGET_PATH)/include && cp -rf $(INSTALL_PATH)/lib64/*.so* $(TARGET_PATH)/lib

clean:
	rm -rf $(INSTALL_PATH)
	rm -rf $(TARGET_PATH)/include/soci
	rm -rf $(TARGET_PATH)/lib/libsoci_*

.PHONY: all install clean
