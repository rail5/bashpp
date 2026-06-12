#!/usr/bin/env bash

set -euo pipefail

parser_directory="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
repository_directory="$(cd "$parser_directory/.." && pwd)"
tree_sitter="$parser_directory/node_modules/.bin/tree-sitter"
source_paths="$(mktemp)"
trap 'rm -f "$source_paths"' EXIT

find \
	"$repository_directory/examples" \
	"$repository_directory/test-suite" \
	"$repository_directory/wiki/_includes/code/snippets" \
	-type f \
	-name '*.bpp' \
	! -name 'parser-errors-1.bpp' \
	! -name 'parser-errors-2.bpp' \
	-print > "$source_paths"

source_count="$(wc -l < "$source_paths")"
if [[ "$source_count" -eq 0 ]]; then
	echo "No Bash++ source files were found." >&2
	exit 1
fi

cd "$parser_directory"
"$tree_sitter" parse --quiet --stat --paths "$source_paths"

for fixture in parser-errors-1.bpp parser-errors-2.bpp; do
	fixture_path="$repository_directory/test-suite/tests/sources/$fixture"
	if "$tree_sitter" parse --quiet "$fixture_path"; then
		echo "Expected $fixture to contain a syntax error." >&2
		exit 1
	fi
done

printf 'Parsed %d valid Bash++ files and verified 2 recovery fixtures.\n' \
	"$source_count"
