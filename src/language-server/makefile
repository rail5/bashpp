STATIC_FILES = $(wildcard static/*.h)
GENERATED_FILES = $(wildcard generated/*.h)

# If GENERATED_FILES is empty, just add a dummy file to ensure everything gets created
ifeq ($(GENERATED_FILES),)
GENERATED_FILES = generated/InitializeRequest.h
endif

all: bpp-lsp

bpp-lsp: main.o BashppServer.o ThreadPool.o
	g++ -O2 -s -std=gnu++17 -o $@ $^

main.o: main.cpp BashppServer.h ThreadPool.h
	g++ -O2 -s -std=gnu++17 -c main.cpp

BashppServer.o: BashppServer.cpp BashppServer.h $(STATIC_FILES) $(GENERATED_FILES)
	g++ -O2 -s -std=gnu++17 -c BashppServer.cpp

ThreadPool.o: ThreadPool.cpp ThreadPool.h
	g++ -O2 -s -std=gnu++17 -c ThreadPool.cpp

generated/%: generateLSPClasses
	mkdir -p generated
	./generateLSPClasses ./metaModel.json ./generated

generateLSPClasses: generateLSPClasses.cpp metaModel.json TypeRegistry.h TypeRegistry.cpp
	g++ -O2 -s -std=gnu++17 -o generateLSPClasses ./generateLSPClasses.cpp

clean:
	rm -f bpp-lsp
	rm -f generateLSPClasses
	rm -rf generated

.PHONY: all clean
.PRECIOUS: generated/%
