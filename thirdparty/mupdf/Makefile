TARGET_PATH := $(PWD)/../../build/

TAR_LIB := mupdf-1.14.0-source.tar.gz
LIB_PATH := mupdf-1.14.0-source
INSTALL_PREFIX := $(PWD)/build/

all:
	tar -zxf $(TAR_LIB) && chmod -R +x $(LIB_PATH)
	mkdir -p $(INSTALL_PREFIX) && cd $(LIB_PATH) && make -f ../mupdf.mk install-lib XCFLAGS=-fPIC build=release HAVE_X11=no HAVE_GLUT=no prefix=$(INSTALL_PREFIX) install

install:
	install -d $(TARGET_PATH)
	
	install -d $(TARGET_PATH)/lib
	install $(INSTALL_PREFIX)/lib/*.so* $(TARGET_PATH)/lib
	
	install -d $(TARGET_PATH)/include
	install -d $(TARGET_PATH)/include/mupdf
	install -d $(TARGET_PATH)/include/mupdf/fitz
	install -d $(TARGET_PATH)/include/mupdf/pdf
	install $(INSTALL_PREFIX)/include/mupdf/*.h $(TARGET_PATH)/include/mupdf
	install $(INSTALL_PREFIX)/include/mupdf/fitz/*.h $(TARGET_PATH)/include/mupdf/fitz
	install $(INSTALL_PREFIX)/include/mupdf/pdf/*.h $(TARGET_PATH)/include/mupdf/pdf
	
clean:
	rm -rf $(LIB_PATH) && rm -rf $(INSTALL_PREFIX)

.PHONY: all clean install
