#!/usr/bin/env bash

for snippet in "./snippets/"*; do
	outputFile=${snippet//.bpp/.html}
	node colorize.js "$snippet" "$outputFile"
done
