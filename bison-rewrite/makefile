BINDIR = bin
OBJDIR = $(BINDIR)/obj
TARGET = $(BINDIR)/bplex

SRCDIR = src
LEXERDIR = $(SRCDIR)/lexer
PARSERDIR = $(SRCDIR)/parser
GENERATEDDIR = $(SRCDIR)/generated

LEXER_SRC = $(LEXERDIR)/lexer.l
LEXER = $(GENERATEDDIR)/lex.yy.cpp

PARSER_SRC = $(PARSERDIR)/parser.y
PARSER = $(GENERATEDDIR)/parser.tab.cpp

CXX = g++
CXXFLAGS = -std=gnu++23 -Wall -Wextra -O2 -s

OBJECTS = $(OBJDIR)/main.o $(OBJDIR)/ModeStack.o $(OBJDIR)/generated/parser.tab.o $(OBJDIR)/generated/lex.yy.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(LEXER): $(LEXER_SRC)
	flex --header-file=$(GENERATEDDIR)/lex.yy.hpp -o $(LEXER) $<

$(PARSER): $(PARSER_SRC)
	bison -o $(PARSER) -d $< -Wcounterexamples

$(SRCDIR)/main.cpp: $(LEXER) $(PARSER)
	@# This target ensures that main.cpp is considered dependent on the generated files

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(LEXER) $(PARSER)
	mkdir -p $(shell dirname $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(GENERATEDDIR)/*
	rm -f $(TARGET)
	rm -rf $(OBJDIR)

.PHONY: all clean
