ANTLR4 = antlr4 # Path to the antlr4 executable
ANTLR4_HEADERS = /usr/include/antlr4-runtime # Path to the antlr4-runtime headers

ANTLR4_RUNTIME_LIBRARY = $(shell (find /usr -name libantlr4-runtime.a || find /usr -name libantlr4-runtime.so) | head -n 1)

CXX = g++
CXXFLAGS = -std=gnu++17 -O2 -s -Wall
INCLUDEFLAGS = -I$(ANTLR4_HEADERS)

BPP_OBJDIR = obj/bpp
ANTLR4_OBJDIR = obj/antlr4

BPP_INCLUDEDIR = bpp_include

BPP_SRCS = $(wildcard $(BPP_INCLUDEDIR)/*.cpp)
BPP_OBJS = $(patsubst $(BPP_INCLUDEDIR)/%.cpp,$(BPP_OBJDIR)/%.o,$(BPP_SRCS))

ANTLR4DIR = antlr

ANTLR4_SRCS = $(wildcard $(ANTLR4DIR)/*.cpp)
ANTLR4_OBJS = $(patsubst $(ANTLR4DIR)/%.cpp,$(ANTLR4_OBJDIR)/%.o,$(ANTLR4_SRCS))

MAIN = main.cpp
MAIN_OBJ = obj/$(MAIN:.cpp=.o)

all: bpp

bpp: $(BPP_OBJS) $(ANTLR4_OBJS) $(MAIN_OBJ)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $(BPP_OBJS) $(ANTLR4_OBJS) $(MAIN_OBJ) $(ANTLR4_RUNTIME_LIBRARY)

$(BPP_OBJDIR)/%.o: $(BPP_INCLUDEDIR)/%.cpp $(ANTLR4_OBJS)
	@mkdir -p $(BPP_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

$(ANTLR4_OBJDIR)/%.o: $(ANTLR4DIR)/%.cpp
	@mkdir -p $(ANTLR4_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

obj/$(MAIN:.cpp=.o): $(MAIN) $(ANTLR4_OBJS)
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

clean:
	@rm -rf obj
	@rm -f $(BPP_OBJDIR)/*.o
	@rm -f $(ANTLR4_OBJDIR)/*.o
	@rm -f obj/*.o
	@rm -f bpp
	@echo "Cleaned up build artifacts."
