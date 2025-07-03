# Default to using all available CPU cores for parallel builds
# unless the user specifies a different number of jobs with -jN
ifeq ($(filter -j%,$(MAKEFLAGS)),)
MAKEFLAGS += -j$(shell nproc)
endif

.SUFFIXES:

PARSECHANGELOG := $(shell command -v dpkg-parsechangelog 2> /dev/null)

ifdef PARSECHANGELOG
	VERSION=$$(dpkg-parsechangelog -l debian/changelog --show-field version)
	LASTUPDATEDYEAR=$$(date +%Y -d@$$(dpkg-parsechangelog -l debian/changelog --show-field timestamp))
else
	VERSION=$$(head -n1 debian/changelog | grep -o "[[:digit:].]*" || echo "0.1")
	LASTUPDATEDYEAR=$$(grep "^ \-- " debian/changelog | head -n 1 | cut -d, -f2 | date -d - +%Y || date +%Y || echo "2025")
endif

# STDLIB_FILES:
# Find all files without extensions in the stdlib directory
# And append ".sh" to each file name
STDLIB_FILES := $(shell find stdlib -type f -not -name '*.*' -exec basename {} \; | sed 's/^/stdlib\//; s/$$/.sh/')

all: compiler std

compiler: clean-main update-version update-year
	cd src && $(MAKE)
	@mv src/bpp bin/bpp

std: clean-std compiler
	@echo $(STDLIB_FILES)
	$(MAKE) $(STDLIB_FILES)

stdlib/%.sh: stdlib/%
	@echo "Compiling stdlib: $<"
	@bin/bpp -o $@ $<

test:
	bin/bpp test-suite/run.bpp

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
	@sed -i 's/# Basic usage/ SYNOPSIS/g' tmp/compiler.md
	@sed -i '1s/^/% bpp(7) Version '"$(VERSION)"' | Manual for the Bash++ language\n/' tmp/language.md
	@sed -i 's/Programming in Bash++/NAME\nbpp - The Bash++ language/g' tmp/language.md
	$(MAKE) process-manual-code-snippets FILE=tmp/language.md
	@pandoc --standalone --to man tmp/compiler.md -o debian/bpp.1
	@pandoc --standalone --to man tmp/language.md -o debian/bpp.7
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

update-version:
	@ \
	if [ ! -z "$(VERSION)" ]; then \
		echo "#define bpp_compiler_version \"$(VERSION)\"" > src/version.h; \
	fi;

update-year:
	@ \
	if [ ! -z "$(LASTUPDATEDYEAR)" ]; then \
		echo "#define bpp_compiler_updated_year \"$(LASTUPDATEDYEAR)\"" > src/updated_year.h; \
	fi;

clean-src:
	@cd src && $(MAKE) clean

clean-main:
	@rm -f bin/bpp

clean-std:
	@rm -f stdlib/*.sh

clean-manual: clean-detailed-manuals
	@rm -f debian/bpp.1
	@rm -f debian/bpp.7
	@rm -rf tmp

clean-detailed-manuals:
	@rm -f debian/bpp-*.3
	@rm -rf detailed-manuals

clean-technical-docs:
	@rm -rf docs

clean: clean-src clean-main clean-std clean-manual clean-technical-docs

.PHONY: all compiler std test process-manual-code-snippets clean-src clean-main clean-std clean-manual clean-detailed-manuals clean-technical-docs clean update-version update-year detailed-manuals manual technical-docs stdlib/%.sh
