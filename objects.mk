include config.mk

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
$(LISTENER_OBJDIR)/%.o: $(SRCDIR)/listener/%.cpp $(HEADERS) $(LISTENERS)
	@mkdir -p $(LISTENER_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

# Rule to compile all .cpp files in the listener/handlers directory
$(HANDLERS_OBJDIR)/%.o: $(SRCDIR)/listener/handlers/%.cpp $(HEADERS)
	@mkdir -p $(HANDLERS_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

# Rule to compile all .cpp files in the lsp directory
$(LSP_OBJDIR)/%.o: $(LSPDIR)/%.cpp $(LSP_GENERATED_FILES)
	@mkdir -p $(LSP_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

$(LSP_GENERATOR_OBJDIR)/%.o: $(LSPDIR)/generator/%.cpp
	@mkdir -p $(LSP_GENERATOR_OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXTRA_OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(EXTRA_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

# Rule to compile the main.cpp file
## The Antlr4 objects are a prerequisite for this
bin/obj/main.o: $(MAIN) $(ANTLR4_OBJS) $(ANTLR4DIR)/BashppParser.cpp $(HEADERS) $(LISTENERS)
	@mkdir -p bin/obj
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

# Rule to compile the bpp-lsp.cpp file
bin/obj/bpp-lsp.o: src/bpp-lsp.cpp $(HEADERS) $(LSP_STATIC_FILES) $(LSP_GENERATED_FILES)
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

clean-objects:
	@rm -rf bin/obj
	@find bin -name '*.d' -exec rm -f {} +
	@echo "Cleaned up object files."

.PHONY: clean-objects
