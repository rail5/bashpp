all:
	npm install
	vsce package

publish:
	vsce publish

clean:
	rm -f *.vsix
	rm -rf node_modules
	rm -f dist/extension.js

.PHONY: all publish clean
