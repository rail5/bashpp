# ANTLR4 CONFIG

# Path to the antlr4 executable
ANTLR4 = antlr4
# Path to the antlr4-runtime headers
ANTLR4_HEADERS = /usr/include/antlr4-runtime # Path to the antlr4-runtime headers
# Location of the antlr4-runtime library
## This prefers the static library if available, otherwise it falls back to the shared library
ANTLR4_RUNTIME_LIBRARY = $(shell (find /usr -name libantlr4-runtime.a || find /usr -name libantlr4-runtime.so) | head -n 1)



# C++ CONFIG

CXX = g++
CXXFLAGS = -std=gnu++17 -O2 -s -Wall -Wno-attributes -MMD -MP
INCLUDEFLAGS = -I$(ANTLR4_HEADERS)


# OBJECT DIRECTORIES
# These directories will be used to store the object files

BPP_OBJDIR = bin/obj/bpp
ANTLR4_OBJDIR = bin/obj/antlr4
LISTENER_OBJDIR = bin/obj/listener
HANDLERS_OBJDIR = bin/obj/listener/handlers
EXTRA_OBJDIR = bin/obj/extra
LSP_OBJDIR = bin/obj/lsp
LSP_HANDLER_OBJDIR = bin/obj/lsp/handlers
LSP_GENERATOR_OBJDIR = bin/obj/lsp/generator


# SOURCES
SRCDIR = src

BPP_INCLUDEDIR = $(SRCDIR)/bpp_include

# List of all .cpp files in the bpp_include directory
# This will be used to generate the object files
BPP_SRCS = $(wildcard $(BPP_INCLUDEDIR)/*.cpp)
BPP_OBJS = $(patsubst $(BPP_INCLUDEDIR)/%.cpp,$(BPP_OBJDIR)/%.o,$(BPP_SRCS))


ANTLR4DIR = $(SRCDIR)/antlr


# List of all .cpp files in the antlr directory
# This will be used to generate the object files
ANTLR4_SRCS = $(wildcard $(ANTLR4DIR)/*.cpp)
ANTLR4_OBJS = $(patsubst $(ANTLR4DIR)/%.cpp,$(ANTLR4_OBJDIR)/%.o,$(ANTLR4_SRCS))


# List of all .cpp files in the listener directory
LISTENER_SRCS = $(wildcard src/listener/*.cpp)
LISTENER_OBJS = $(patsubst src/listener/%.cpp,$(LISTENER_OBJDIR)/%.o,$(LISTENER_SRCS))


# List of all .cpp files in the src/listener/handlers directory
# This will be used to generate the object files for the handlers
HANDLER_SRCS = $(wildcard src/listener/handlers/*.cpp)
HANDLER_OBJS = $(patsubst src/listener/handlers/%.cpp,$(HANDLERS_OBJDIR)/%.o,$(HANDLER_SRCS))


EXTRA_SRCS = $(SRCDIR)/internal_error.cpp \
			$(SRCDIR)/syntax_error.cpp
EXTRA_OBJS = $(patsubst $(SRCDIR)/%.cpp,$(EXTRA_OBJDIR)/%.o,$(EXTRA_SRCS))


MAIN = $(SRCDIR)/main.cpp
MAIN_OBJ = bin/obj/main.o


HEADERS = $(wildcard $(BPP_INCLUDEDIR)/*.h) \
		  $(SRCDIR)/listener/BashppListener.h


LISTENERS = $(wildcard src/listener/handlers/*.cpp)


# LANGUAGE SERVER CONFIG

LSPDIR = $(SRCDIR)/lsp

LSP_STATIC_FILES = $(wildcard $(LSPDIR)/static/*.h)
LSP_GENERATED_FILES = $(wildcard $(LSPDIR)/generated/*.h)

# If GENERATED_FILES is empty, just add a dummy file to ensure everything gets created
ifeq ($(LSP_GENERATED_FILES),)
LSP_GENERATED_FILES = $(LSPDIR)/generated/InitializeRequest.h
endif

LSP_SRCS = $(wildcard $(LSPDIR)/*.cpp)
LSP_OBJS = $(patsubst $(LSPDIR)/%.cpp,$(LSP_OBJDIR)/%.o,$(LSP_SRCS))

LSP_GENERATOR_SRCS = $(wildcard $(LSPDIR)/generator/*.cpp)
LSP_GENERATOR_OBJS = $(patsubst $(LSPDIR)/generator/%.cpp,$(LSP_GENERATOR_OBJDIR)/%.o,$(LSP_GENERATOR_SRCS))

LSP_HANDLER_SRCS = $(wildcard $(LSPDIR)/handlers/*.cpp)
LSP_HANDLER_OBJS = $(patsubst $(LSPDIR)/handlers/%.cpp,$(LSP_HANDLER_OBJDIR)/%.o,$(LSP_HANDLER_SRCS))
