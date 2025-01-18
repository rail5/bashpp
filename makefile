all: clean
	cd src && make
	mv src/bpp bin/bpp

parser:
	cd src && make parser
	mv src/BashppParser bin/BashppParser

manual:
	mkdir tmp
	tail -n +6 wiki/language.md > tmp/language.md
	cp wiki/compiler.md tmp/
	sed -i '1s/^/% bpp(1) Version 1.0 | Manual for the Bash++ compiler\n/' tmp/compiler.md
	sed -i 's/Using the Bash++ compiler/NAME\nbpp - compiler for the Bash++ language/g' tmp/compiler.md
	sed -i 's/# Basic usage/ SYNOPSIS/g' tmp/compiler.md
	sed -i '1s/^/% bpp(5) Version 1.0 | Manual for the Bash++ language\n/' tmp/language.md
	sed -i 's/Programming in Bash++/NAME\nbpp - The Bash++ language/g' tmp/language.md
	pandoc --standalone --to man tmp/compiler.md -o debian/bpp.1
	pandoc --standalone --to man tmp/language.md -o debian/bpp.5
	rm -rf tmp

clean:
	cd src && make clean
	rm -f bin/*
	rm -rf tmp
	rm -f debian/bpp.1
	rm -f debian/bpp.5