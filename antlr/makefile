all:
	antlr4 -Dlanguage=Cpp ./BashppLexer.g4 ./BashppParser.g4 -o ./out
	#g++ -std=c++17 -O2 -I/usr/include/antlr4-runtime -o BashppParser main.cpp out/BashppLexer.cpp out/BashppParser.cpp -L/usr/lib -lantlr4-runtime
	g++ -std=c++17 -O2 -I/usr/include/antlr4-runtime -o bpp bpp.cpp out/BashppLexer.cpp out/BashppParser.cpp -L/usr/lib -lantlr4-runtime

clean:
	rm -rf out
	rm -f BashppParser
	rm -rf .antlr
	rm -f bpp
