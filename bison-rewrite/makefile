LEXER_SRC = lexer.l
LEXER = lex.yy.c
PARSER_SRC = parser.y
PARSER = parser.tab.cc
TARGET = bplex

CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O2 -g

all: $(PARSER) $(LEXER) main.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^

$(LEXER): $(LEXER_SRC)
	flex $<

$(PARSER): $(PARSER_SRC)
	bison -d $< -Wcounterexamples

clean:
	rm -f $(TARGET) $(PARSER) parser.tab.hh $(LEXER)
