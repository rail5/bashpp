# RULES FOR BUILDING DOCUMENTATION

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

manual: clean-manual detailed-manuals
	@mkdir tmp
	@tail -n +6 wiki/language.md > tmp/language.md
	@cp wiki/compiler.md tmp/
	@sed -i '1s/^/% bpp(1) Version '"$(VERSION)"' | Manual for the Bash++ compiler\n/' tmp/compiler.md
	@sed -i 's/Using the Bash++ compiler/NAME\nbpp - Compiler for the Bash++ language/g' tmp/compiler.md
	@sed -i 's/Basic usage/SYNOPSIS/g' tmp/compiler.md
	@sed -i '1s/^/% bpp(7) Version '"$(VERSION)"' | Manual for the Bash++ language\n/' tmp/language.md
	@sed -i 's/Programming in Bash++/NAME\nbpp - The Bash++ language/g' tmp/language.md
	$(MAKE) process-manual-code-snippets FILE=tmp/language.md
	@tail -n +5 "wiki/bpp-lsp.md" > tmp/bpp-lsp.md
	@sed -i '1s/^/% bpp-lsp(1) Version '"$(VERSION)"' | Bash++ Language Server\n/' tmp/bpp-lsp.md
	@pandoc --standalone --to man tmp/compiler.md -o debian/bpp.1
	@pandoc --standalone --to man tmp/language.md -o debian/bpp.7
	@pandoc --standalone --to man tmp/bpp-lsp.md -o debian/bpp-lsp.1
	@rm -rf tmp

detailed-manuals: clean-detailed-manuals
	@mkdir detailed-manuals
	@for file in "wiki/spec/"*.md; do \
		if [ "$$(basename "$$file")" = "index.md" ]; then \
			continue; \
		fi; \
		base=$$(basename "$$file" .md); \
		tail -n +5 "$$file" > "detailed-manuals/bpp-$$base.md"; \
		sed -i '1s/^/% bpp-'"$$base"'(3) Version '"$(VERSION)"' | Manual for the Bash++ language\n/' "detailed-manuals/bpp-$$base.md"; \
		$(MAKE) process-manual-code-snippets FILE="detailed-manuals/bpp-$$base.md"; \
	done
	@for file in "detailed-manuals/"*.md; do \
		base=$$(basename "$$file" .md); \
		pandoc --standalone --to man "$$file" -o "debian/$$base.3"; \
	done
	@rm -rf detailed-manuals

technical-docs: clean-technical-docs
	doxygen Doxyfile

clean-manual: clean-detailed-manuals
	@rm -f debian/*.1
	@rm -f debian/*.7
	@rm -rf tmp
	@echo "Cleaned up manuals."

clean-detailed-manuals:
	@rm -f debian/bpp-*.3
	@rm -rf detailed-manuals

clean-technical-docs:
	@rm -rf docs
	@echo "Cleaned up technical documentation."

.PHONY: process-manual-code-snippets manual detailed-manuals technical-docs clean-manual clean-detailed-manuals clean-technical-docs
