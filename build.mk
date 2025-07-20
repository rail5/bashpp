compiler-and-lsp:
	@echo "Generating necessary files..."
	@$(MAKE) --no-print-directory src/antlr/BashppParser.cpp src/lsp/generated/.stamp
	@echo "Building the compiler and language server..."
	@$(MAKE) --no-print-directory bin/bpp bin/bpp-lsp

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
bin/bpp: $(BPP_OBJS) $(ANTLR4_OBJS) $(LISTENER_OBJS) $(HANDLER_OBJS) $(EXTRA_OBJS) $(MAIN_OBJ) $(ANTLR4DIR)/BashppParser.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $(BPP_OBJS) $(ANTLR4_OBJS) $(LISTENER_OBJS) $(HANDLER_OBJS) $(EXTRA_OBJS) $(MAIN_OBJ) $(ANTLR4_RUNTIME_LIBRARY)

bin/bpp-lsp: bin/obj/bpp-lsp.o $(LSP_OBJS) $(ANTLR4_OBJS) $(BPP_OBJS) $(LISTENER_OBJS) $(HANDLER_OBJS) $(EXTRA_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $^ $(ANTLR4_RUNTIME_LIBRARY)

.PHONY: compiler-and-lsp

ifeq ($(filter clean%,$(MAKECMDGOALS)),)
-include $(shell find obj -name '*.d' 2>/dev/null)
endif
