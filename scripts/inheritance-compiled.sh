#!/usr/bin/env bash

function bpp__Animal____new() {
	local objectName="$1" setPtr="$2"
	if [[ "$objectName" == "" ]]; then
		while : ; do
			objectName="$RANDOM$RANDOM$RANDOM$RANDOM"
			local exemplaryDataMember="bpp__Animal__${objectName}__height"
			[[ -z ${!exemplaryDataMember+x} ]] && break
		done
	fi
	eval "bpp__Animal__${objectName}__height=\"60cm\""
	eval "bpp__Animal__${objectName}__weight=\"30kg\""
	if [[ ! -z "$setPtr" ]]; then
		eval "$setPtr=\"bpp__Animal__${objectName}\""
	fi
}

function bpp__Animal____delete() {
	local objectName="$1" isPtr="$2"
	local objectAddress="bpp__Animal__${objectName}"
	if [[ "$isPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	eval "unset ${objectAddress}__height"
	eval "unset ${objectAddress}__weight"
}

function bpp__Animal____copy() {
	local copyFrom="$1" copyTo="$2" copyFromIsPtr="$3" copyToIsPtr="$4"
	local copyFrom__height="bpp__Animal__${copyFrom}__height" copyTo__height="bpp__Animal__${copyTo}__height"
	local copyFrom__weight="bpp__Animal__${copyFrom}__weight" copyTo__weight="bpp__Animal__${copyTo}__weight"
	if [[ "$copyFromIsPtr" -eq 1 ]]; then
		copyFrom__height="${copyFrom}__height"
		copyFrom__weight="${copyFrom}__weight"
	fi
	if [[ "$copyToIsPtr" -eq 1 ]]; then
		copyTo__height="${copyTo}__height"
		copyTo__weight="${copyTo}__weight"
	fi
	eval "$copyTo__height=\"${!copyFrom__height}\""
	eval "$copyTo__weight=\"${!copyFrom__weight}\""
}

function bpp__Animal__toPrimitive() {
	echo "Animal Instance"
}

function bpp__Animal__updateHeight__primitive() {
	if [[ $# -ne 3 ]]; then
		echo "Bash++: Invalid number of arguments passed to @Animal.updateHeight"
		echo "(Expected 1, got $(($# - 2)))"
	fi
	local objectName="$1" objectIsPtr="$2" height="$3"
	local objectAddress="bpp__Animal__${objectName}"
	if [[ "$objectIsPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__height="${objectAddress}__height"
	eval "${this__height}=\"$height\""
}

function bpp__Animal__updateWeight__primitive() {
	if [[ $# -ne 3 ]]; then
		echo "Bash++: Invalid number of arguments passed to @Animal.updateWeight"
		echo "(Expected 1, got $(($# - 2)))"
	fi
	local objectName="$1" objectIsPtr="$2" weight="$3"
	local objectAddress="bpp__Animal__${objectName}"
	if [[ "$objectIsPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__weight="${objectAddress}__weight"
	eval "${this__weight}=\"$weight\""
}

function bpp__Animal__pet() {
	if [[ $# -ne 2 ]]; then
		echo "Bash++: Invalid number of arguments passed to @Animal.pet"
		echo "(Expected 0, got $(($# - 2)))"
	fi
	local objectName="$1" objectIsPtr="$2"
	local objectAddress="bpp__Animal__${objectName}"
	if [[ "$objectIsPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__height="${objectAddress}__height" this__weight="${objectAddress}__weight"
	echo "You extend your hand to pet it, but the ${!this__height}, ${!this__weight} beast bares its teeth"
}

function bpp__Dog__pet() {
	echo "You pet the dog"
}

function bpp__Lion____constructor() {
	local objectName="$1" objectIsPtr="$2"
	bpp__Animal__updateHeight__primitive "${objectName}" "${objectIsPtr}" "120cm"
	bpp__Animal__updateWeight__primitive "${objectName}" "${objectIsPtr}" "200kg"
}

function bpp__Lion__pet() {
	if [[ $# -ne 2 ]]; then
		echo "Bash++: Invalid number of arguments passed to @Lion.pet"
		echo "(Expected 0, got $(($# - 2)))"
	fi
	local objectName="$1" objectIsPtr="$2"
	local objectAddress="bpp__Animal__${objectName}"
	if [[ "$objectIsPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__weight="${objectAddress}__weight"
	echo "You are devoured by the lion"
	echo "...Why did you try to pet a ${!this__weight} lion?"
}

bpp__Animal____new "animal"
bpp__Animal__pet "animal" 0

bpp__Animal____new "dog"
bpp__Dog__pet "dog" 0

bpp__Animal____new "lion1"
bpp__Lion____constructor "lion1" 0
bpp__Lion__pet "lion1" 0

bpp__ptr__Lion__lion2="bpp__Animal__animal"
bpp__Lion__pet "${bpp__ptr__Lion__lion2}" 1

