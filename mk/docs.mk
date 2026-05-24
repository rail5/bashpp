# RULES FOR BUILDING DOCUMENTATION

SPEC_SRCS            := $(filter-out wiki/spec/index.md, $(wildcard wiki/spec/*.md))
# wiki/spec/*.md -> debian/bpp-*.3
SPEC_MANPAGES        := $(patsubst wiki/spec/%.md, debian/bpp-%.3, $(SPEC_SRCS))

COMPILER_MANPAGE_SRC := wiki/compiler.md
COMPILER_MANPAGE     := debian/bpp.1

LSP_MANPAGE_SRC      := wiki/bpp-lsp.md
LSP_MANPAGE          := debian/bpp-lsp.1

LANGUAGE_MANPAGE_SRC := wiki/language.md
LANGUAGE_MANPAGE     := debian/bpp.7

MANPAGES := $(SPEC_MANPAGES) $(COMPILER_MANPAGE) $(LSP_MANPAGE) $(LANGUAGE_MANPAGE)

manpages: $(MANPAGES)

bin/man/3/bpp-%.md: wiki/spec/%.md
	@mkdir -p $(@D)
	@tail -n +7 $< > $@ # Remove the first 6 lines (YAML front matter)
	@sed -i '1s/^/% bpp-$*(3) Version '"$(VERSION)"' | Manual for the Bash++ language\n/' $@ # Add manpage header
	@$(MAKE) process-manual-code-snippets FILE=$@ # Replace Jekyll includes with actual content and convert code snippets to markdown format

debian/bpp-%.3: bin/man/3/bpp-%.md
	@mkdir -p $(@D)
	@pandoc --standalone --to man $< -o $@



bin/man/1/bpp.md: wiki/compiler.md
	@mkdir -p $(@D)
	@cp $< $@
	@sed -i '1s/^/% bpp(1) Version '"$(VERSION)"' | Manual for the Bash++ compiler\n/' $@ # Add manpage header
	@sed -i 's/Using the Bash++ compiler/NAME\nbpp - Compiler for the Bash++ language/g' $@ # Replace section header
	@sed -i 's/Basic usage/SYNOPSIS/g' $@ # Replace section header

bin/man/1/bpp-lsp.md: wiki/bpp-lsp.md
	@mkdir -p $(@D)
	@tail -n +5 $< > $@ # Remove the first 4 lines (YAML front matter)
	@sed -i '1s/^/% bpp-lsp(1) Version '"$(VERSION)"' | Bash++ Language Server\n/' $@ # Add manpage header

debian/%.1: bin/man/1/%.md
	@mkdir -p $(@D)
	@pandoc --standalone --to man $< -o $@



bin/man/7/bpp.md: wiki/language.md
	@mkdir -p $(@D)
	@tail -n +6 $< > $@ # Remove the first 5 lines (YAML front matter)
	@sed -i '1s/^/% bpp(7) Version '"$(VERSION)"' | Manual for the Bash++ language\n/' $@ # Add manpage header
	@sed -i 's/Programming in Bash++/NAME\nbpp - The Bash++ language/g' $@ # Replace section header
	@$(MAKE) process-manual-code-snippets FILE=$@ # Replace Jekyll includes with actual content and convert code snippets to markdown format

debian/%.7: bin/man/7/%.md
	@mkdir -p $(@D)
	@pandoc --standalone --to man $< -o $@


technical-docs: clean-technical-docs
	doxygen Doxyfile

process-manual-code-snippets:
	@if [ -z "$(FILE)" ]; then \
		echo "Error: FILE variable is not set. Please provide a file to process."; \
		exit 1; \
	fi
	@echo "Processing included code snippets in $(FILE)..."
	@while grep -oq '{%- include' $(FILE); do \
		file=$$(grep -o '{%- include \(.*\) -%}' $(FILE) | head -n 1 | sed 's/{%- include \(.*\) -%}/\1/'); \
		bppFile=$$(echo $$file | sed 's/.html/.bpp/'); \
		content=$$(cat wiki/_includes/$$bppFile); \
		perl -pe 'BEGIN {undef $$/; $$file = q{\{%- include '"$$file"' -%\}}; $$content = q{'"$$content"'};} s/\Q$$file\E/$$content/g' $(FILE) > $(FILE).tmp && mv $(FILE).tmp $(FILE); \
	done
	@sed -i 's/<div class="highlight"><pre class="highlight"><code>/```bash/g' $(FILE)
	@sed -i 's/<\/code><\/pre><\/div>/```/g' $(FILE)


clean-technical-docs:
	@rm -rf docs
	@echo "Cleaned technical documentation."

clean-spec-manpages:
	@rm -f debian/bpp-*.3
	@rm -rf bin/man/3
	@echo "Cleaned spec manpages."

clean-manpages: clean-spec-manpages
	@rm -f debian/*.1
	@rm -f debian/*.7
	@rm -rf bin/man
	@echo "Cleaned all manpages."

.PHONY: technical-docs manpages process-manual-code-snippets clean-technical-docs clean-spec-manpages clean-manpages

.PRECIOUS: bin/man/3/bpp-%.md bin/man/1/%.md bin/man/7/%.md
