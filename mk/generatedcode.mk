include mk/config.mk

# FLEX/BISON LEXER/PARSER GENERATION
FLEXBISON_LOCK = .flexbison.lock

$(FLEXBISON_GENERATED_STAMP): $(FLEXBISONDIR)/lexer.l $(FLEXBISONDIR)/parser.y
	@if [ ! -f $(FLEXBISON_LOCK) ]; then \
		touch $(FLEXBISON_LOCK); \
		echo "Generating lexer and parser..."; \
		mkdir -p $(FLEXBISON_GENERATEDDIR); \
		flex --header-file=$(FLEXBISON_GENERATEDDIR)/lex.yy.hpp -o $(FLEXBISON_GENERATEDDIR)/lex.yy.cpp $(FLEXBISONDIR)/lexer.l; \
		bison -o $(FLEXBISON_GENERATEDDIR)/parser.tab.cpp -d $(FLEXBISONDIR)/parser.y; \
		touch $@; \
		rm -f $(FLEXBISON_LOCK); \
	else \
		while [ -f $(FLEXBISON_LOCK) ]; do \
			sleep 0.1; \
		done; \
	fi

$(FLEXBISON_GENERATEDDIR)/%.cpp: $(FLEXBISON_GENERATED_STAMP)
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
	@rm -rf $(FLEXBISON_GENERATEDDIR)
	@rm -f $(FLEXBISON_LOCK)
	@echo "Cleaned up Flex/Bison-generated files."
clean-lsp:
	@rm -f $(wildcard $(LSP_GENERATEDDIR)/*)
	@echo "Cleaned up LSP-generated files."

clean-meta:
	@rm -f $(SRCDIR)/version.h $(SRCDIR)/updated_year.h
	@echo "Cleaned up meta files."

.PHONY: clean-flexbison clean-lsp clean-meta
