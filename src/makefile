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
CXXFLAGS = -std=gnu++17 -O2 -s -Wall
INCLUDEFLAGS = -I$(ANTLR4_HEADERS)

BPP_OBJDIR = obj/bpp
ANTLR4_OBJDIR = obj/antlr4

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

MAIN = main.cpp
MAIN_OBJ = obj/$(MAIN:.cpp=.o)

HEADERS = $(wildcard $(BPP_INCLUDEDIR)/*.h) \
		  listener/BashppListener.h

LISTENERS = $(wildcard listener/handlers/*.cpp)

all: $(ANTLR4DIR)/BashppParser.cpp
	$(MAKE) bpp

# Rule to link all object files into the final executable
## Prerequisites:
## - All .cpp files in the bpp_include directory
## - All .cpp files in the antlr directory
## - The main.cpp file
bpp: $(BPP_OBJS) $(ANTLR4_OBJS) $(MAIN_OBJ) $(ANTLR4DIR)/BashppParser.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $(BPP_OBJS) $(ANTLR4_OBJS) $(MAIN_OBJ) $(ANTLR4_RUNTIME_LIBRARY)

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

# Rule to compile the main.cpp file
## The Antlr4 objects are a prerequisite for this
obj/$(MAIN:.cpp=.o): $(MAIN) $(ANTLR4_OBJS) $(ANTLR4DIR)/BashppParser.cpp $(HEADERS) $(LISTENERS)
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

$(ANTLR4DIR)/%.cpp: BashppLexer.g4 BashppParser.g4
	@mkdir -p $(ANTLR4DIR)
	$(ANTLR4) -Dlanguage=Cpp ./BashppLexer.g4 ./BashppParser.g4 -o ./antlr

clean:
	@rm -rf obj
	@rm -f $(BPP_OBJDIR)/*.o
	@rm -f $(ANTLR4_OBJDIR)/*.o
	@rm -rf antlr
	@rm -rf .antlr
	@rm -f obj/*.o
	@rm -f bpp
	@echo "Cleaned up build artifacts."
