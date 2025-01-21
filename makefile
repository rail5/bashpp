VERSION=$$(dpkg-parsechangelog -l debian/changelog --show-field version)
LASTUPDATEDYEAR=$$(date +%Y -d@$$(dpkg-parsechangelog -l debian/changelog --show-field timestamp))

all: clean-src clean-main update-version update-year
	cd src && make
	mv src/bpp bin/bpp

sll: clean-src
	cd src && make sll
	mv src/bpp-sll bin/bpp-sll

parser: clean-parser
	cd src && make parser
	mv src/BashppParser bin/BashppParser

manual: clean-manual
	mkdir tmp
	tail -n +6 wiki/language.md > tmp/language.md
	cp wiki/compiler.md tmp/
	sed -i '1s/^/% bpp(1) Version '"$(VERSION)"' | Manual for the Bash++ compiler\n/' tmp/compiler.md
	sed -i 's/Using the Bash++ compiler/NAME\nbpp - compiler for the Bash++ language/g' tmp/compiler.md
	sed -i 's/# Basic usage/ SYNOPSIS/g' tmp/compiler.md
	sed -i '1s/^/% bpp(5) Version '"$(VERSION)"' | Manual for the Bash++ language\n/' tmp/language.md
	sed -i 's/Programming in Bash++/NAME\nbpp - The Bash++ language/g' tmp/language.md
	pandoc --standalone --to man tmp/compiler.md -o debian/bpp.1
	pandoc --standalone --to man tmp/language.md -o debian/bpp.5
	rm -rf tmp

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
	cd src && make clean

clean-main:
	rm -f bin/bpp

clean-sll:
	rm -f bin/bpp-sll

clean-parser:
	rm -f bin/BashppParser

clean-manual:
	rm -f debian/bpp.1
	rm -f debian/bpp.5
	rm -rf tmp

clean: clean-src clean-main clean-parser clean-manual