include mk/config.mk

# ANTLR4 PARSER GENERATION
ANTLR4_LOCK = .antlr4.lock

$(ANTLR4_STAMP): $(SRCDIR)/BashppLexer.g4 $(SRCDIR)/BashppParser.g4
	@if [ ! -f $(ANTLR4_LOCK) ]; then \
		touch $(ANTLR4_LOCK); \
		echo "Generating ANTLR4 parser..."; \
		mkdir -p $(ANTLR4DIR); \
		cd $(SRCDIR); \
		$(ANTLR4) -Dlanguage=Cpp BashppLexer.g4 BashppParser.g4 -o antlr; \
		cd - >/dev/null; \
		touch $@; \
		rm -f $(ANTLR4_LOCK); \
	else \
		while [ -f $(ANTLR4_LOCK) ]; do \
			sleep 0.1; \
		done; \
	fi

$(ANTLR4DIR)/%.cpp: $(ANTLR4_STAMP)
	@:

# LSP CLASSES GENERATION
$(LSP_OBJDIR)/generateLSPClasses: $(LSP_GENERATOR_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $^

$(LSP_GENERATED_STAMP): $(LSP_OBJDIR)/generateLSPClasses
	@mkdir -p $(LSP_GENERATEDDIR)
	$< "$(LSPDIR)/metaModel.json" "$(LSP_GENERATEDDIR)"
	@touch $@


# META HEADER FILES GENERATION (VERSION NUMBER, UPDATED YEAR)
$(SRCDIR)/version.h:
	@if [ ! -z "$(VERSION)" ]; then \
		echo "#define bpp_compiler_version \"$(VERSION)\"" > $@; \
	else \
		echo "#define bpp_compiler_version \"0.1\"" > $@; \
	fi

$(SRCDIR)/updated_year.h:
	@if [ ! -z "$(YEAR)" ]; then \
		echo "#define bpp_compiler_updated_year \"$(YEAR)\"" > $@; \
	else \
		echo "#define bpp_compiler_updated_year \"2025\"" > $@; \
	fi

clean-antlr:
	@rm -rf $(ANTLR4DIR)
	@rm -rf $(SRCDIR)/.antlr
	@rm -f $(ANTLR4_LOCK)
	@echo "Cleaned up ANTLR4-generated files."

clean-lsp:
	@rm -f $(wildcard $(LSP_GENERATEDDIR)/*)
	@echo "Cleaned up LSP-generated files."

clean-meta:
	@rm -f $(SRCDIR)/version.h $(SRCDIR)/updated_year.h
	@echo "Cleaned up meta files."

.PHONY: clean-antlr clean-lsp clean-meta
