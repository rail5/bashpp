# ANTLR4 CONFIG
ANTLR4 = antlr4
ANTLR4_HEADERS = /usr/include/antlr4-runtime
ANTLR4_RUNTIME_LIBRARY = $(shell (find /usr -name libantlr4-runtime.a || find /usr -name libantlr4-runtime.so) 2>/dev/null | head -n 1)

# C++ CONFIG
CXX = g++
CXXFLAGS = -std=gnu++23 -O2 -s -Wall -MMD -MP
INCLUDEFLAGS = -isystem $(ANTLR4_HEADERS)

# Can we parse debian/changelog?
PARSECHANGELOG := $(shell command -v dpkg-parsechangelog 2>/dev/null)

ifdef PARSECHANGELOG
	VERSION = $(shell dpkg-parsechangelog -l debian/changelog --show-field version)
	YEAR = $(shell date +%Y -d@$(shell dpkg-parsechangelog -l debian/changelog --show-field timestamp))
else
	VERSION = $(shell head -n1 debian/changelog | grep -o "[[:digit:].]*" || echo "0.1")
	YEAR = $(shell grep "^ \-- " debian/changelog | head -n 1 | cut -d, -f2 | date -d +%Y || date +%Y || echo "2025")
endif

# DIRECTORIES FOR COMPILED FILES
BINDIR = bin
OBJDIR = $(BINDIR)/obj

BPP_OBJDIR = $(OBJDIR)/bpp
ANTLR4_OBJDIR = $(OBJDIR)/antlr4
LISTENER_OBJDIR = $(OBJDIR)/listener
COMPILER_HANDLERS_OBJDIR = $(LISTENER_OBJDIR)/handlers
EXTRA_OBJDIR = $(OBJDIR)/extra

LSP_OBJDIR = $(OBJDIR)/lsp
LSP_INCLUDE_OBJDIR = $(LSP_OBJDIR)/include
LSP_HANDLERS_OBJDIR = $(LSP_OBJDIR)/handlers
LSP_GENERATOR_OBJDIR = $(LSP_OBJDIR)/generator

# SOURCES
SRCDIR = src

BPP_INCLUDEDIR = $(SRCDIR)/bpp_include
BPP_SRCS = $(wildcard $(BPP_INCLUDEDIR)/*.cpp)
BPP_OBJS = $(patsubst $(BPP_INCLUDEDIR)/%.cpp,$(BPP_OBJDIR)/%.o,$(BPP_SRCS))
BPP_HEADERS = $(wildcard $(BPP_INCLUDEDIR/*.h))

LISTENER_SRCDIR = $(SRCDIR)/listener
LISTENER_SRCS = $(wildcard $(LISTENER_SRCDIR)/*.cpp)
LISTENER_OBJS = $(patsubst $(LISTENER_SRCDIR)/%.cpp,$(LISTENER_OBJDIR)/%.o,$(LISTENER_SRCS))
LISTENER_HEADER = $(wildcard $(LISTENER_SRCDIR)/*.h)

COMPILER_HANDLERS_SRCDIR = $(LISTENER_SRCDIR)/handlers
COMPILER_HANDLERS_SRCS = $(wildcard $(COMPILER_HANDLERS_SRCDIR)/*.cpp)
COMPILER_HANDLERS_OBJS = $(patsubst $(COMPILER_HANDLERS_SRCDIR)/%.cpp,$(COMPILER_HANDLERS_OBJDIR)/%.o,$(COMPILER_HANDLERS_SRCS))

EXTRA_SRCS = $(SRCDIR)/internal_error.cpp $(SRCDIR)/syntax_error.cpp
EXTRA_OBJS = $(patsubst $(SRCDIR)/%.cpp,$(EXTRA_OBJDIR)/%.o,$(EXTRA_SRCS))

MAIN = $(SRCDIR)/main.cpp
MAIN_OBJ = $(OBJDIR)/main.o

LSPDIR = $(SRCDIR)/lsp
LSP_SRCS = $(wildcard $(LSPDIR)/*.cpp)
LSP_OBJS = $(patsubst $(LSPDIR)/%.cpp,$(LSP_OBJDIR)/%.o,$(LSP_SRCS))

LSP_STATICDIR = $(LSPDIR)/static
LSP_STATIC_FILES = $(wildcard $(LSP_STATICDIR)/*.h)

LSP_INCLUDEDIR = $(LSPDIR)/include
LSP_INCLUDE_SRCS = $(wildcard $(LSP_INCLUDEDIR)/*.cpp)
LSP_INCLUDE_OBJS = $(patsubst $(LSP_INCLUDEDIR)/%.cpp,$(LSP_INCLUDE_OBJDIR)/%.o,$(LSP_INCLUDE_SRCS))

LSP_HANDLERDIR = $(LSPDIR)/handlers
LSP_HANDLERS_SRCS = $(wildcard $(LSP_HANDLERDIR)/*.cpp)
LSP_HANDLERS_OBJS = $(patsubst $(LSP_HANDLERDIR)/%.cpp,$(LSP_HANDLERS_OBJDIR)/%.o,$(LSP_HANDLERS_SRCS))

LSP_GENERATORDIR = $(LSPDIR)/generator
LSP_GENERATOR_SRCS = $(wildcard $(LSP_GENERATORDIR)/*.cpp)
LSP_GENERATOR_OBJS = $(patsubst $(LSP_GENERATORDIR)/%.cpp,$(LSP_GENERATOR_OBJDIR)/%.o,$(LSP_GENERATOR_SRCS))

LSP_MAIN = $(SRCDIR)/bpp-lsp.cpp
LSP_MAIN_OBJ = $(OBJDIR)/bpp-lsp.o

# GENERATED CODE LOCATIONS
ANTLR4DIR = $(SRCDIR)/antlr
ANTLR4_STAMP = $(ANTLR4DIR)/.antlr4.stamp
ANTLR4_SRCS = $(ANTLR4DIR)/BashppParser.cpp $(ANTLR4DIR)/BashppLexer.cpp
ANTLR4_OBJS = $(patsubst $(ANTLR4DIR)/%.cpp,$(ANTLR4_OBJDIR)/%.o,$(ANTLR4_SRCS))

LSP_GENERATEDDIR = $(LSPDIR)/generated
LSP_GENERATED_STAMP = $(LSP_GENERATEDDIR)/.stamp
