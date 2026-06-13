# Tree-sitter Bash++

This directory contains the Tree-sitter parser for Bash++. It extends the
upstream [`tree-sitter-bash`](https://github.com/tree-sitter/tree-sitter-bash)
0.25.1 grammar so that Bash syntax remains owned and tested by the Bash parser.
The parser is kept in this repository because the compiler sources, examples,
and documentation snippets are its compatibility suite.

## Development

From the repository root, install the locked dependencies and generate the
tracked parser sources:

```sh
make tree-sitter
```

Run generation, linting, corpus tests, and repository-wide parsing:

```sh
make test-tree-sitter
```

Generated parser sources under `src/` are build artifacts and are not committed.
Run `make tree-sitter` before compiling the parser or packaging an editor
integration.

The corpus is grouped by Bash compatibility, declarations, expressions,
Bash interactions, and error recovery. Repository parsing covers every `.bpp`
file under `examples/`, `test-suite/`, and `wiki/_includes/code/snippets/`.
`parser-errors-1.bpp` and `parser-errors-2.bpp` are intentionally malformed and
are asserted separately as recovery cases.

## Grammar Interface

Named nodes and fields in `grammar.js` are the parser's public interface.
They cover classes and members, object and pointer declarations, references,
assignments, allocation and casts, supershells, and Bash++ interpolation in
Bash constructs. Future highlighting queries and editor integrations should
consume these nodes rather than infer structure from source text.

The external scanner is inherited from `tree-sitter-bash` and adapted only for
Bash++ tokens that require lookahead, such as declaration types, assignment
references, reference boundaries, and heredoc interpolation.

## Scope and Licensing

This milestone contains only the parser. It intentionally excludes highlighting
queries, editor configuration, language bindings, and language-server support.

New parser work is licensed under GPL-3.0-or-later. Adapted
`tree-sitter-bash` material retains its MIT notice under
`THIRD_PARTY_LICENSES/`.
