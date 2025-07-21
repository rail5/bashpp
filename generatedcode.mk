include config.mk

# Rules for generated C++ code
# (The ANTLR4-generated parser, the LSP classes)

# Rule to generate the LSP classes from the metaModel.json file
$(LSP_GENERATED_FILES): $(LSPDIR)/generated/.stamp

$(LSPDIR)/generated/.stamp: bin/obj/lsp/generateLSPClasses
	@mkdir -p $(LSPDIR)/generated
	bin/obj/lsp/generateLSPClasses "$(LSPDIR)/metaModel.json" "$(LSPDIR)/generated"
	@touch $@


$(ANTLR4_STAMP): $(SRCDIR)/BashppLexer.g4 $(SRCDIR)/BashppParser.g4
	@mkdir -p $(ANTLR4DIR)
	cd $(SRCDIR) && $(ANTLR4) -Dlanguage=Cpp ./BashppLexer.g4 ./BashppParser.g4 -o antlr
	touch $@

$(ANTLR4DIR)/BashppParser.cpp \
$(ANTLR4DIR)/BashppParser.h   \
$(ANTLR4DIR)/BashppLexer.cpp  \
$(ANTLR4DIR)/BashppLexer.h : $(ANTLR4_STAMP)

# Rule to generate the LSP classes executable
bin/obj/lsp/generateLSPClasses: $(LSP_GENERATOR_OBJS)
	@mkdir -p $(LSP_GENERATOR_OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -o $@ $^

update-version:
	@ \
	if [ ! -z "$(VERSION)" ]; then \
		echo "#define bpp_compiler_version \"$(VERSION)\"" > src/version.h; \
	else \
		echo "#define bpp_compiler_version \"0.5.2\"" > src/version.h; \
	fi;

update-year:
	@ \
	if [ ! -z "$(LASTUPDATEDYEAR)" ]; then \
		echo "#define bpp_compiler_updated_year \"$(LASTUPDATEDYEAR)\"" > src/updated_year.h; \
	else \
		echo "#define bpp_compiler_updated_year \"2025\"" > src/updated_year.h; \
	fi;

clean-antlr:
	@rm -rf $(ANTLR4DIR)
	@echo "Cleaned up ANTLR4 generated files."

clean-lsp:
	@rm -f $(LSP_GENERATED_FILES)
	@echo "Cleaned up LSP generated files."

.PHONY: clean-antlr clean-lsp
