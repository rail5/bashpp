# C++ CONFIG
CXX = g++
CXXFLAGS = -std=gnu++23 -O2 -s -Wall -MMD -MP
INCLUDEFLAGS = -Isrc/

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
FLEXBISON_OBJDIR = $(OBJDIR)/flexbison
FLEXBISON_GENERATED_OBJDIR = $(FLEXBISON_OBJDIR)/generated
LISTENER_OBJDIR = $(OBJDIR)/listener
COMPILER_HANDLERS_OBJDIR = $(LISTENER_OBJDIR)/handlers
AST_OBJDIR = $(OBJDIR)/AST
ERROR_OBJDIR = $(OBJDIR)/error

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

AST_SRCDIR = $(SRCDIR)/AST
AST_SRCS = $(wildcard $(AST_SRCDIR)/*.cpp)
AST_OBJS = $(patsubst $(AST_SRCDIR)/%.cpp,$(AST_OBJDIR)/%.o,$(AST_SRCS))

FLEXBISONDIR = $(SRCDIR)/flexbison
FLEXBISON_SRCS = $(wildcard $(FLEXBISONDIR)/*.cpp)
FLEXBISON_OBJS = $(patsubst $(FLEXBISONDIR)/%.cpp,$(FLEXBISON_OBJDIR)/%.o,$(FLEXBISON_SRCS))

ERROR_SRCS = $(wildcard $(SRCDIR)/error/*.cpp)
ERROR_OBJS = $(patsubst $(SRCDIR)/error/%.cpp,$(ERROR_OBJDIR)/%.o,$(ERROR_SRCS))

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
FLEXBISON_GENERATEDDIR = $(SRCDIR)/flexbison/generated
FLEXBISON_GENERATED_STAMP = $(FLEXBISON_GENERATEDDIR)/.stamp
FLEXBISON_GENERATED_SRCS = $(FLEXBISON_GENERATEDDIR)/parser.tab.cpp $(FLEXBISON_GENERATEDDIR)/lex.yy.cpp
FLEXBISON_GENERATED_OBJS = $(patsubst $(FLEXBISON_GENERATEDDIR)/%.cpp,$(FLEXBISON_GENERATED_OBJDIR)/%.o,$(FLEXBISON_GENERATED_SRCS))

LSP_GENERATEDDIR = $(LSPDIR)/generated
LSP_GENERATED_STAMP = $(LSP_GENERATEDDIR)/.stamp
