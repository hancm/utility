TARGET_PATH := $(PWD)/../../build/

INCLUDE_INSTALL_PREFIX := /usr/include/oracle/11.2/client64/
LIB_INSTALL_PREFIX := /usr/lib/oracle/11.2/client64/lib/

all:
	rpm -ivh --force oracle-instantclient11.2-basic-11.2.0.4.0-1.x86_64.rpm
	rpm -ivh --force oracle-instantclient11.2-devel-11.2.0.4.0-1.x86_64.rpm
	rpm -ivh --force oracle-instantclient11.2-sqlplus-11.2.0.4.0-1.x86_64.rpm
	mkdir -p $(TARGET_PATH)/include/occi && cp -rf $(INCLUDE_INSTALL_PREFIX)/* $(TARGET_PATH)/include/occi 
	cp -rf $(LIB_INSTALL_PREFIX)/* $(TARGET_PATH)/lib

clean:
	rm -rf $(TARGET_PATH)/include/occi && rm -rf $(TARGET_PATH)/lib/occi

.PHONY: all clean
