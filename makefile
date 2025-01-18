all:
	cd src && make
	mv src/bpp bin/bpp

debug:
	cd src && make debug
	mv src/BashppParser bin/BashppParser

clean:
	cd src && make clean
	rm -f bin/*
