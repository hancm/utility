include Makefile

$(OUT)/libmupdf.so: 
	g++ $(CFLAGS) -shared -o $@ $(MUPDF_OBJ) $(THIRD_OBJ)  

shared-lib:	$(RM_CMD) $(THIRD_OBJ) $(MUPDF_OBJ) $(OUT)/libmupdf.so	

install-lib: shared-lib
	install -d $(DESTDIR)$(incdir)/mupdf
	install -d $(DESTDIR)$(incdir)/mupdf/fitz
	install -d $(DESTDIR)$(incdir)/mupdf/pdf
	install include/mupdf/*.h $(DESTDIR)$(incdir)/mupdf
	install include/mupdf/fitz/*.h $(DESTDIR)$(incdir)/mupdf/fitz
	install include/mupdf/pdf/*.h $(DESTDIR)$(incdir)/mupdf/pdf

	install -d $(DESTDIR)$(libdir)
	install $(OUT)/libmupdf.so $(DESTDIR)$(libdir)

