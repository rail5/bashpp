all:
	g++ -O2 -s -std=gnu++17 -o bpp-lsp ./main.cpp

generateLSPClasses: generateLSPClasses.cpp metaModel.json TypeRegistry.h TypeRegistry.cpp
	g++ -O2 -s -std=gnu++17 -o generateLSPClasses ./generateLSPClasses.cpp

lsp: generateLSPClasses
	./generateLSPClasses

clean:
	rm -f bpp-lsp
