# Tree-sitter Bash++

This directory contains the Tree-sitter parser for Bash++. It extends the
upstream [`tree-sitter-bash`](https://github.com/tree-sitter/tree-sitter-bash)
grammar so that Bash syntax remains owned and tested by the Bash parser.

## Development

Install the pinned development dependencies and generate the parser:

```sh
npm install
npm run generate
```

Run the grammar lint and corpus tests:

```sh
npm run lint
npm test
```

Generated parser sources under `src/` are committed so editor integrations can
build the parser without installing Node.js dependencies.
