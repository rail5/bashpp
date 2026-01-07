BINDIR = bin

SRCDIR = src
LEXERDIR = $(SRCDIR)/lexer
PARSERDIR = $(SRCDIR)/parser
GENERATEDDIR = $(SRCDIR)/generated

LEXER_SRC = $(LEXERDIR)/lexer.l
LEXER = $(GENERATEDDIR)/lex.yy.c

PARSER_SRC = $(PARSERDIR)/parser.y
PARSER = $(GENERATEDDIR)/parser.tab.cc

TARGET = $(BINDIR)/bplex

CXX = g++
CXXFLAGS = -std=gnu++23 -Wall -Wextra -O2 -g


all: $(TARGET)

$(TARGET): $(PARSER) $(LEXER) $(SRCDIR)/main.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^

$(LEXER): $(LEXER_SRC)
	flex -o $(LEXER) $<

$(PARSER): $(PARSER_SRC)
	bison -o $(PARSER) -d $< -Wcounterexamples

clean:
	rm -f $(GENERATEDDIR)/*
	rm -f $(BINDIR)/*
