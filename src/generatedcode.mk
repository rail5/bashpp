include config.mk

# Rules for generated C++ code
# (The ANTLR4-generated parser, the LSP classes)

# Rule to generate the LSP classes from the metaModel.json file
$(LSP_GENERATED_FILES): $(LSPDIR)/generated/.stamp

$(LSPDIR)/generated/.stamp: obj/lsp/generateLSPClasses
	@mkdir -p $(LSPDIR)/generated
	obj/lsp/generateLSPClasses "$(LSPDIR)/metaModel.json" "$(LSPDIR)/generated"
	@touch $@


$(ANTLR4DIR)/%.cpp: BashppLexer.g4 BashppParser.g4
	@mkdir -p $(ANTLR4DIR)
	$(ANTLR4) -Dlanguage=Cpp ./BashppLexer.g4 ./BashppParser.g4 -o ./antlr


# Rule to generate the LSP classes executable
obj/lsp/generateLSPClasses: $(LSP_GENERATOR_OBJS)
	@mkdir -p $(LSP_GENERATOR_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $^

clean-antlr:
	@rm -rf $(ANTLR4DIR)
	@echo "Cleaned up ANTLR4 generated files."

clean-lsp:
	@rm -f $(LSP_GENERATED_FILES)
	@echo "Cleaned up LSP generated files."

.PHONY: clean-antlr clean-lsp
