BUILD_LIB_DIRS := openssl soci podofo mupdf civetweb tacopie cpp_redis occi

third:
	@$(foreach lib, $(BUILD_LIB_DIRS), \
		cd (lib) && make && make install; \
	)

clean:
	@$(foreach lib, $(BUILD_LIB_DIRS), \
		cd (lib) && make clean; \
	)

.PHONY: third clean
