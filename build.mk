compiler-and-lsp:
	@$(MAKE) --no-print-directory compiler lsp

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

compiler:
	@echo "Generating ANTLR parser..."
	@$(MAKE) --no-print-directory src/antlr/BashppParser.cpp
	@$(MAKE) --no-print-directory bin/bpp

bin/bpp-lsp: bin/obj/bpp-lsp.o $(LSP_OBJS) $(ANTLR4_OBJS) $(BPP_OBJS) $(LISTENER_OBJS) $(HANDLER_OBJS) $(EXTRA_OBJS) $(LSP_HANDLER_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $^ $(ANTLR4_RUNTIME_LIBRARY)

lsp:
	@echo "Generating LSP classes..."
	@$(MAKE) --no-print-directory src/lsp/generated/.stamp
	@$(MAKE) --no-print-directory bin/bpp-lsp

.PHONY: compiler-and-lsp compiler lsp

ifeq ($(filter clean%,$(MAKECMDGOALS)),)
-include $(shell find bin -name '*.d' 2>/dev/null)
endif
