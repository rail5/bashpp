all:
	g++ -O2 -s -o bpp main.cpp -std=gnu++20

debug:
	g++ -g -o bpp main.cpp -std=gnu++20

clean:
	rm -f ./bpp
