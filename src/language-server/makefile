all: generated/LSPTypes.h
	g++ -O2 -s -std=gnu++17 -o bpp-lsp ./main.cpp

generated/%: generateLSPClasses
	mkdir -p generated
	./generateLSPClasses ./metaModel.json ./generated

generateLSPClasses: generateLSPClasses.cpp metaModel.json TypeRegistry.h TypeRegistry.cpp
	g++ -O2 -s -std=gnu++17 -o generateLSPClasses ./generateLSPClasses.cpp

clean:
	rm -f bpp-lsp
	rm -f generateLSPClasses
	rm -rf generated
