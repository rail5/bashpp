# Default to using all available CPU cores for parallel builds
# unless the user specifies a different number of jobs with -jN
ifeq ($(filter -j%,$(MAKEFLAGS)),)
MAKEFLAGS += -j$(shell nproc)
endif

all:
	@echo "Generating necessary files..."
	@$(MAKE) --no-print-directory antlr/BashppParser.cpp lsp/generated/.stamp
	@echo "Building the compiler and language server..."
	@$(MAKE) --no-print-directory bpp bpp-lsp

# Standard config -- where to find antlr4, g++, build directories, source directories, etc
include config.mk

# Rules for building .o files
include objects.mk

# Rules for generating code
include generatedcode.mk

# Rule to link all object files into the final executable
## Prerequisites:
## - All .cpp files in the bpp_include directory
## - All .cpp files in the antlr directory
## - The main.cpp file
bpp: $(BPP_OBJS) $(ANTLR4_OBJS) $(LISTENER_OBJS) $(HANDLER_OBJS) $(EXTRA_OBJS) $(MAIN_OBJ) $(ANTLR4DIR)/BashppParser.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $(BPP_OBJS) $(ANTLR4_OBJS) $(LISTENER_OBJS) $(HANDLER_OBJS) $(EXTRA_OBJS) $(MAIN_OBJ) $(ANTLR4_RUNTIME_LIBRARY)

bpp-lsp: obj/bpp-lsp.o $(LSP_OBJS) $(ANTLR4_OBJS) $(BPP_OBJS) $(LISTENER_OBJS) $(HANDLER_OBJS) $(EXTRA_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $^ $(ANTLR4_RUNTIME_LIBRARY)

clean: clean-objects clean-antlr clean-lsp
	@rm -rf .antlr
	@rm -f bpp
	@rm -f bpp-lsp
	@echo "Cleaned up build artifacts."

.PHONY: all clean

ifeq ($(filter clean clean-objects clean-antlr clean-lsp,$(MAKECMDGOALS)),)
-include $(shell find obj -name '*.d' 2>/dev/null)
endif
