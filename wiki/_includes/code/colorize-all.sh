#!/usr/bin/env bash

counter=0
max_jobs=5

for snippet in "./snippets/"*; do
	outputFile=${snippet//.bpp/.html}
	node colorize.js "$snippet" "$outputFile" &
	((counter++))
	if ((counter % max_jobs == 0)); then
		wait
	fi
done

wait
