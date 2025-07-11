# Default to using all available CPU cores for parallel builds
# unless the user specifies a different number of jobs with -jN
ifeq ($(filter -j%,$(MAKEFLAGS)),)
MAKEFLAGS += -j$(shell nproc)
endif

# Path to the antlr4 executable
ANTLR4 = antlr4
# Path to the antlr4-runtime headers
ANTLR4_HEADERS = /usr/include/antlr4-runtime # Path to the antlr4-runtime headers
# Location of the antlr4-runtime library
## This prefers the static library if available, otherwise it falls back to the shared library
ANTLR4_RUNTIME_LIBRARY = $(shell (find /usr -name libantlr4-runtime.a || find /usr -name libantlr4-runtime.so) | head -n 1)

CXX = g++
CXXFLAGS = -std=gnu++17 -O2 -s -Wall -MMD -MP
INCLUDEFLAGS = -I$(ANTLR4_HEADERS)

BPP_OBJDIR = obj/bpp
ANTLR4_OBJDIR = obj/antlr4
LISTENER_OBJDIR = obj/listener
HANDLERS_OBJDIR = obj/listener/handlers
EXTRA_OBJDIR = obj/extra
LSP_OBJDIR = obj/lsp
LSP_GENERATOR_OBJDIR = obj/lsp/generator

BPP_INCLUDEDIR = bpp_include

# List of all .cpp files in the bpp_include directory
# This will be used to generate the object files
BPP_SRCS = $(wildcard $(BPP_INCLUDEDIR)/*.cpp)
BPP_OBJS = $(patsubst $(BPP_INCLUDEDIR)/%.cpp,$(BPP_OBJDIR)/%.o,$(BPP_SRCS))

ANTLR4DIR = antlr

# List of all .cpp files in the antlr directory
# This will be used to generate the object files
ANTLR4_SRCS = $(wildcard $(ANTLR4DIR)/*.cpp)
ANTLR4_OBJS = $(patsubst $(ANTLR4DIR)/%.cpp,$(ANTLR4_OBJDIR)/%.o,$(ANTLR4_SRCS))

# List of all .cpp files in the listener directory
LISTENER_SRCS = $(wildcard listener/*.cpp)
LISTENER_OBJS = $(patsubst listener/%.cpp,$(LISTENER_OBJDIR)/%.o,$(LISTENER_SRCS))

# List of all .cpp files in the src/listener/handlers directory
# This will be used to generate the object files for the handlers
HANDLER_SRCS = $(wildcard listener/handlers/*.cpp)
HANDLER_OBJS = $(patsubst listener/handlers/%.cpp,$(HANDLERS_OBJDIR)/%.o,$(HANDLER_SRCS))

EXTRA_SRCS = internal_error.cpp \
			syntax_error.cpp
EXTRA_OBJS = $(patsubst %.cpp,$(EXTRA_OBJDIR)/%.o,$(EXTRA_SRCS))

MAIN = main.cpp
MAIN_OBJ = obj/$(MAIN:.cpp=.o)

HEADERS = $(wildcard $(BPP_INCLUDEDIR)/*.h) \
		  listener/BashppListener.h

LISTENERS = $(wildcard listener/handlers/*.cpp)

LSPDIR = lsp

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

all: $(ANTLR4DIR)/BashppParser.cpp
	$(MAKE) bpp bpp-lsp

# Rule to link all object files into the final executable
## Prerequisites:
## - All .cpp files in the bpp_include directory
## - All .cpp files in the antlr directory
## - The main.cpp file
bpp: $(BPP_OBJS) $(ANTLR4_OBJS) $(LISTENER_OBJS) $(HANDLER_OBJS) $(EXTRA_OBJS) $(MAIN_OBJ) $(ANTLR4DIR)/BashppParser.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $(BPP_OBJS) $(ANTLR4_OBJS) $(LISTENER_OBJS) $(HANDLER_OBJS) $(EXTRA_OBJS) $(MAIN_OBJ) $(ANTLR4_RUNTIME_LIBRARY)

bpp-lsp: obj/bpp-lsp.o $(LSP_OBJS)
	@echo $(LSP_OBJS)
	g++ -O2 -s -std=gnu++17 -o $@ $^

# Rule to compile all .cpp files in the bpp_include directory
## Antlr4 objects are a prerequisite for this
$(BPP_OBJDIR)/%.o: $(BPP_INCLUDEDIR)/%.cpp $(ANTLR4_OBJS) $(ANTLR4DIR)/BashppParser.cpp $(HEADERS)
	@mkdir -p $(BPP_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

# Rule to compile all .cpp files in the antlr directory
## The Antlr4-generated files are a prerequisite for this
$(ANTLR4_OBJDIR)/%.o: $(ANTLR4DIR)/%.cpp $(ANTLR4DIR)/BashppParser.cpp $(HEADERS)
	@mkdir -p $(ANTLR4_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

# Rule to compile all .cpp files in the listener directory
$(LISTENER_OBJDIR)/%.o: listener/%.cpp $(HEADERS) $(LISTENERS)
	@mkdir -p $(LISTENER_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

# Rule to compile all .cpp files in the listener/handlers directory
$(HANDLERS_OBJDIR)/%.o: listener/handlers/%.cpp $(HEADERS)
	@mkdir -p $(HANDLERS_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

# Rule to compile all .cpp files in the lsp directory
$(LSP_OBJDIR)/%.o: $(LSPDIR)/%.cpp $(LSP_GENERATED_FILES)
	@mkdir -p $(LSP_OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(LSP_GENERATOR_OBJDIR)/%.o: $(LSPDIR)/generator/%.cpp
	@mkdir -p $(LSP_GENERATOR_OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXTRA_OBJDIR)/%.o: %.cpp
	@mkdir -p $(EXTRA_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

# Rule to generate the LSP classes from the metaModel.json file
$(LSPDIR)/generated/%: generateLSPClasses
	@mkdir -p $(LSPDIR)/generated
	./generateLSPClasses $(LSPDIR)/metaModel.json $(LSPDIR)/generated

# Rule to generate the LSP classes executable
generateLSPClasses: $(LSP_GENERATOR_OBJS)
	@mkdir -p $(LSP_GENERATOR_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $^

# Rule to compile the main.cpp file
## The Antlr4 objects are a prerequisite for this
obj/$(MAIN:.cpp=.o): $(MAIN) $(ANTLR4_OBJS) $(ANTLR4DIR)/BashppParser.cpp $(HEADERS) $(LISTENERS)
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

# Rule to compile the bpp-lsp.cpp file
obj/bpp-lsp.o: bpp-lsp.cpp $(HEADERS) $(LSP_STATIC_FILES) $(LSP_GENERATED_FILES)
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

$(ANTLR4DIR)/%.cpp: BashppLexer.g4 BashppParser.g4
	@mkdir -p $(ANTLR4DIR)
	$(ANTLR4) -Dlanguage=Cpp ./BashppLexer.g4 ./BashppParser.g4 -o ./antlr

clean:
	@rm -rf obj
	@rm -f $(LSP_GENERATED_FILES)
	@rm -rf antlr
	@rm -rf .antlr
	@rm -f obj/*.o
	@find . -name '*.d' -exec rm -f {} +
	@rm -f generateLSPClasses
	@rm -f bpp
	@rm -f bpp-lsp
	@echo "Cleaned up build artifacts."

-include $(shell find obj -name '*.d' 2>/dev/null)
