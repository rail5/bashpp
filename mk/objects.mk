include mk/config.mk

$(BPP_OBJDIR)/%.o: $(BPP_INCLUDEDIR)/%.cpp $(BPP_HEADERS) $(LISTENER_HEADER) $(ANTLR4_STAMP)
	@mkdir -p $(BPP_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

$(ANTLR4_OBJDIR)/%.o: $(ANTLR4DIR)/%.cpp $(BPP_HEADERS) $(LISTENER_HEADER)
	@mkdir -p $(ANTLR4_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

$(LISTENER_OBJDIR)/%.o: $(LISTENER_SRCDIR)/%.cpp $(BPP_HEADERS) $(LISTENER_HEADER) $(ANTLR4_STAMP)
	@mkdir -p $(LISTENER_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

$(COMPILER_HANDLERS_OBJDIR)/%.o: $(COMPILER_HANDLERS_SRCDIR)/%.cpp $(BPP_HEADERS) $(LISTENER_HEADER) $(ANTLR4_STAMP)
	@mkdir -p $(COMPILER_HANDLERS_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

$(EXTRA_OBJDIR)/%.o: $(SRCDIR)/%.cpp $(ANTLR4_STAMP)
	@mkdir -p $(EXTRA_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

$(MAIN_OBJ): $(MAIN) $(ANTLR4_STAMP) $(ANTLR4_SRCS) $(BPP_HEADERS) $(LISTENER_HEADER) $(SRCDIR)/version.h $(SRCDIR)/updated_year.h
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@


$(LSP_OBJDIR)/%.o: $(LSPDIR)/%.cpp $(LSP_GENERATED_STAMP) $(ANTLR4_STAMP)
	@mkdir -p $(LSP_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

$(LSP_INCLUDE_OBJDIR)/%.o: $(LSP_INCLUDEDIR)/%.cpp $(LSP_GENERATED_STAMP) $(ANTLR4_STAMP) $(BPP_HEADERS) $(LISTENER_HEADER)
	@mkdir -p $(LSP_INCLUDE_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

$(LSP_HANDLERS_OBJDIR)/%.o: $(LSP_HANDLERDIR)/%.cpp $(LSP_GENERATED_STAMP) $(ANTLR4_STAMP) $(BPP_HEADERS) $(LISTENER_HEADER)
	@mkdir -p $(LSP_HANDLERS_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

$(LSP_GENERATOR_OBJDIR)/%.o: $(LSP_GENERATORDIR)/%.cpp $(LSPDIR)/metaModel.json
	@mkdir -p $(LSP_GENERATOR_OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/bpp-lsp.o: $(SRCDIR)/bpp-lsp.cpp $(ANTLR4_STAMP) $(LSP_GENERATED_STAMP) $(BPP_HEADERS) $(LISTENER_HEADER) $(LSP_STATIC_FILES) $(SRCDIR)/version.h $(SRCDIR)/updated_year.h
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@


clean-objects:
	@rm -rf $(OBJDIR)
	@find $(BINDIR) -name '*.d' -exec rm -f {} +
	@echo "Cleaned up object files."

.PHONY: clean-objects
