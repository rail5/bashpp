ANTLR4 = antlr4 # Path to the antlr4 executable
ANTLR4_HEADERS = /usr/include/antlr4-runtime # Path to the antlr4-runtime headers

ANTLR4_RUNTIME_LIBRARY = $(shell (find /usr -name libantlr4-runtime.a && find /usr -name libantlr4-runtime.so) | head -n 1)

CXX = g++
CXXFLAGS = -std=gnu++17 -O2 -s -Wall
INCLUDEFLAGS = -I$(ANTLR4_HEADERS)

all: lexer parser main
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o bpp obj/lexer.o obj/parser.o obj/main.o $(ANTLR4_RUNTIME_LIBRARY)

lexer: antlr4
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c -o obj/lexer.o antlr/BashppLexer.cpp

parser: antlr4
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c -o obj/parser.o antlr/BashppParser.cpp

main: antlr4
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c -o obj/main.o main.cpp

antlr4:
	$(ANTLR4) -Dlanguage=Cpp ./BashppLexer.g4 ./BashppParser.g4 -o ./antlr

clean-antlr4:
	rm -rf antlr
	rm -rf .antlr

clean-objects:
	rm -f obj/*.o

clean-bpp:
	rm -f bpp

clean: clean-antlr4 clean-objects clean-bpp
