#!/usr/bin/env bash

function bpp_ExampleClass_initialize() {
	if [[ $# -ne 2 ]]; then
		echo "Bash++: Invalid number of arguments passed to @ExampleClass.initialize"
		echo "(Expected 1, got $(($# - 1)))"
		exit 1
	fi
	local objectName="$1" value="$2"
	local property1Var="bpp_ExampleClass_${objectName}_property1"
	
	eval "${property1Var}=\"$value\""
}

bpp_ExampleClass_initialize "instantiation" "abc def ghi quote doesn't end here\\\" but now it does"

echo $bpp_ExampleClass_instantiation_property1
echo "example @ escape"
echo "another \
example \
@\
string"

