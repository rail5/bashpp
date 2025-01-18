all: antlr4
	g++ -std=gnu++17 -O2 -I/usr/include/antlr4-runtime -o bpp bpp.cpp antlr/BashppLexer.cpp antlr/BashppParser.cpp -L/usr/lib -lantlr4-runtime

debug: antlr4
	g++ -std=gnu++17 -O2 -I/usr/include/antlr4-runtime -o BashppParser main.cpp antlr/BashppLexer.cpp antlr/BashppParser.cpp -L/usr/lib -lantlr4-runtime

antlr4:
	antlr4 -Dlanguage=Cpp ./BashppLexer.g4 ./BashppParser.g4 -o ./antlr

clean:
	rm -rf antlr
	rm -rf .antlr
	rm -f BashppParser
	rm -f bpp
