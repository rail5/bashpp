CXX = g++
CXXFLAGS = -std=gnu++17 -O2
INCLUDEFLAGS = -I/usr/include/antlr4-runtime
ANTLR4_PATH = $(shell find /usr/lib -name libantlr4-runtime.a | head -n 1)

all: antlr4
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o bpp main.cpp antlr/BashppLexer.cpp antlr/BashppParser.cpp $(ANTLR4_PATH)

antlr4:
	antlr4 -Dlanguage=Cpp ./BashppLexer.g4 ./BashppParser.g4 -o ./antlr

clean-antlr4:
	rm -rf antlr
	rm -rf .antlr

clean-bpp:
	rm -f bpp

clean: clean-antlr4 clean-bpp