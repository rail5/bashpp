.PHONY: all dependencies generate test clean

all: generate

dependencies: node_modules/.package-lock.json

node_modules/.package-lock.json: package-lock.json
	npm ci

generate: dependencies
	npm run generate

test: dependencies
	npm run generate
	npm run lint
	npm test
	npm run test:repository

clean:
	rm -rf node_modules .cache src/grammar.json src/node-types.json src/parser.c src/tree_sitter
