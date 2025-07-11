STATIC_FILES = $(wildcard static/*.h)
GENERATED_FILES = $(wildcard generated/*.h)

# If GENERATED_FILES is empty, just add a dummy file to ensure everything gets created
ifeq ($(GENERATED_FILES),)
GENERATED_FILES = generated/InitializeRequest.h
endif

all: bpp-lsp

bpp-lsp: obj/main.o obj/BashppServer.o obj/ThreadPool.o
	g++ -O2 -s -std=gnu++17 -o $@ $^

obj/BashppServer.o: BashppServer.cpp BashppServer.h $(STATIC_FILES) $(GENERATED_FILES)
	g++ -O2 -s -std=gnu++17 -c BashppServer.cpp -o obj/BashppServer.o

obj/main.o: main.cpp BashppServer.h ThreadPool.h $(STATIC_FILES) ../version.h ../updated_year.h
	g++ -O2 -s -std=gnu++17 -c main.cpp -o $@

obj/%.o: %.cpp %.h
	@mkdir -p obj
	g++ -O2 -s -std=gnu++17 -c $< -o $@

generated/%: generateLSPClasses
	@mkdir -p generated
	./generateLSPClasses ./metaModel.json ./generated

generateLSPClasses: generateLSPClasses.cpp metaModel.json obj/TypeRegistry.o
	g++ -O2 -s -std=gnu++17 -o $@ generateLSPClasses.cpp obj/TypeRegistry.o

../version.h:
	@cd ../.. && $(MAKE) update-version

../updated_year.h:
	@cd ../.. && $(MAKE) update-year

clean:
	@rm -f bpp-lsp
	@rm -f generateLSPClasses
	@rm -f generated/*.h
	@rm -f obj/*.o
	@echo "Cleaned up build artifacts."

.PHONY: all clean
.PRECIOUS: generated/%
