include mk/config.mk

# FLEX/BISON LEXER/PARSER GENERATION
FLEXBISON_LOCK = .flexbison.lock

$(FLEXBISON_STAMP): $(SRCDIR)/lexer.l $(SRCDIR)/parser.y
	@if [ ! -f $(FLEXBISON_LOCK) ]; then \
		touch $(FLEXBISON_LOCK); \
		echo "Generating lexer and parser..."; \
		mkdir -p $(FLEXBISONDIR); \
		flex --header-file=$(FLEXBISONDIR)/lex.yy.hpp -o $(FLEXBISONDIR)/lex.yy.cpp src/lexer.l; \
		bison -o $(FLEXBISONDIR)/parser.tab.cpp -d src/parser.y; \
		touch $@; \
		rm -f $(FLEXBISON_LOCK); \
	else \
		while [ -f $(FLEXBISON_LOCK) ]; do \
			sleep 0.1; \
		done; \
	fi

$(FLEXBISONDIR)/%.cpp: $(FLEXBISON_STAMP)
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

clean-flexbison:
	@rm -rf $(FLEXBISONDIR)
	@rm -f $(FLEXBISON_LOCK)
	@echo "Cleaned up Flex/Bison-generated files."
clean-lsp:
	@rm -f $(wildcard $(LSP_GENERATEDDIR)/*)
	@echo "Cleaned up LSP-generated files."

clean-meta:
	@rm -f $(SRCDIR)/version.h $(SRCDIR)/updated_year.h
	@echo "Cleaned up meta files."

.PHONY: clean-flexbison clean-lsp clean-meta
