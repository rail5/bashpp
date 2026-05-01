# C++ CONFIG
CXX      ?= g++
CXXFLAGS ?= -O2 -s
# С++ required flags
CXXFLAGS += -std=gnu++23 -Wall -MMD -MP
# standard preprocessor and linker variables
CPPFLAGS ?=
CPPFLAGS += -Isrc/

# Can we parse debian/changelog?
PARSECHANGELOG := $(shell command -v dpkg-parsechangelog 2>/dev/null)

ifdef PARSECHANGELOG
	VERSION := $(shell dpkg-parsechangelog -l debian/changelog --show-field version)
	YEAR    := $(shell date +%Y -d@$(shell dpkg-parsechangelog -l debian/changelog --show-field timestamp))
else
	VERSION := $(shell head -n1 debian/changelog | grep -o '[[:digit:].]*' || echo "0.1")
	YEAR    := $(shell date +%Y --date="$(shell grep '^ -- ' debian/changelog | head -n 1 | cut -d, -f2 || date +%s)" || date +%Y || echo "2025")
endif

# DIRECTORIES FOR COMPILED FILES
BINDIR                 := bin
OBJDIR                 := $(BINDIR)/obj
SRCDIR                 := src

FLEXBISONDIR           := $(SRCDIR)/flexbison
FLEXBISON_GENERATEDDIR := $(FLEXBISONDIR)/generated

LSPDIR                 := $(SRCDIR)/lsp
LSP_GENERATORDIR       := $(LSPDIR)/generator

# Object output roots (used by generatedcode.mk)
LSP_OBJDIR             := $(OBJDIR)/lsp

# Entrypoints
MAIN                   := $(SRCDIR)/main.cpp
MAIN_OBJ               := $(OBJDIR)/main.o

LSP_MAIN               := $(SRCDIR)/bpp-lsp.cpp
LSP_MAIN_OBJ           := $(OBJDIR)/bpp-lsp.o

# SOURCE DISCOVERY
#
# Derive object lists by walking the source tree and mirroring paths into
# $(OBJDIR). This keeps the make configuration resilient to adding/removing
# subdirectories.

# All sources needed for the compiler (excludes the LSP subtree and entrypoints).
# Flex/Bison generated sources are handled explicitly below.
BPP_SRCS := $(shell find $(SRCDIR)/*/ -type f -name '*.cpp' \
	! -path '$(LSPDIR)/*' \
	! -path '$(FLEXBISON_GENERATEDDIR)/*')
BPP_OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(BPP_SRCS))

# LSP sources (excluding the generator, which builds the codegen tool).
LSP_SRCS := $(shell find $(LSPDIR) -type f -name '*.cpp' \
	! -path '$(LSP_GENERATORDIR)/*')
LSP_OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(LSP_SRCS))

# LSP generator sources (builds the generateLSPClasses host tool).
LSP_GENERATOR_SRCS := $(shell find $(LSP_GENERATORDIR) -type f -name '*.cpp')
LSP_GENERATOR_OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(LSP_GENERATOR_SRCS))

# GENERATED CODE LOCATIONS
FLEXBISON_GENERATED_STAMP   := $(FLEXBISON_GENERATEDDIR)/.stamp
FLEXBISON_GENERATED_SRCS    := $(FLEXBISON_GENERATEDDIR)/parser.tab.cpp $(FLEXBISON_GENERATEDDIR)/lex.yy.cpp
FLEXBISON_GENERATED_HEADERS := $(FLEXBISON_GENERATEDDIR)/parser.tab.hpp $(FLEXBISON_GENERATEDDIR)/lex.yy.hpp
FLEXBISON_GENERATED_OBJS    := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(FLEXBISON_GENERATED_SRCS))

LSP_GENERATEDDIR    := $(LSPDIR)/generated
LSP_GENERATED_STAMP := $(LSP_GENERATEDDIR)/.stamp
