all:
	npm install
	vsce package

publish:
	vsce publish

clean:
	rm -f *.vsix
	rm -rf node_modules

.PHONY: all publish clean
