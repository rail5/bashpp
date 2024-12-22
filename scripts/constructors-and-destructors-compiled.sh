#!/usr/bin/env bash

function bpp__Person____new() {
	local objectName="$1" setPtr="$2"
	if [[ "$objectName" == "" ]]; then
		while : ; do
			objectName="$RANDOM$RANDOM$RANDOM$RANDOM"
			local exemplaryDataMember="bpp__Person__${objectName}__name"
			[[ -z ${!exemplaryDataMember+x} ]] && break
		done
	fi
	eval "bpp__Person__${objectName}__name=\"\""
	if [[ ! -z "$setPtr" ]]; then
		eval "$setPtr=\"bpp__Person__${objectName}\""
	fi
}

function bpp__Person____delete() {
	local objectName="$1" isPtr="$2"
	local objectAddress="bpp__Person__${objectName}"
	if [[ "$isPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	eval "unset ${objectAddress}__name"
}

function bpp__Person____copy() {
	local copyFrom="$1" copyTo="$2" copyFromIsPtr="$3" copyToIsPtr="$4"
	local copyFrom__name="bpp__Person__${copyFrom}__name" copyTo__name="bpp__Person__${copyTo}__name"
	if [[ "$copyFromIsPtr" -eq 1 ]]; then
		copyFrom__name="${copyFrom}__name"
	fi
	if [[ "$copyToIsPtr" -eq 1 ]]; then
		copyTo__name="${copyTo}__name"
	fi
	eval "$copyTo__name=\"${!copyFrom__name}\""
}

function bpp__Person__toPrimitive() {
	echo "Person Instance"
}

function bpp__Person____constructor() {
	local objectName="$1" objectIsPtr="$2"
	local objectAddress="bpp__Person__${objectName}"
	if [[ "$isPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__name="${objectAddress}__name"
	eval "${this__name}=\"John Q. Public\""
	echo "First constructor called: ${!this__name}" >> constructors-and-destructors.log
}

function bpp__Person____constructor__primitive() {
	local objectName="$1" objectIsPtr="$2" name="$3"
	local objectAddress="bpp__Person__${objectName}"
	if [[ "$isPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__name="${objectAddress}__name"
	eval "${this__name}=\"$name\""
	echo "Second constructor called: ${!this__name}" >> constructors-and-destructors.log
}

function bpp__Person____constructor__Person() {
	local objectName="$1" objectIsPtr="$2" otherPerson="$3"
	local objectAddress="bpp__Person__${objectName}"
	if [[ "$isPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__name="${objectAddress}__name"
	local otherPerson__name="${otherPerson}__name"
	eval "${this__name}=\"${!otherPerson__name}\""
	echo "Third constructor called: ${!this__name}" >> constructors-and-destructors.log
}

function bpp__Person____destructor() {
	local objectName="$1" objectIsPtr="$2"
	local objectAddress="bpp__Person__${objectName}"
	if [[ "$isPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__name="${objectAddress}__name"
	echo "Destructing ${!this__name}" >> constructors-and-destructors.log
}

bpp__Person____new "john"
bpp__Person____constructor "john" 0

bpp__Person____new "mike"
bpp__Person____constructor__primitive "mike" 0 "Michael J. Fox"

function bpp__ScopeDemonstration____new() {
	local objectName="$1" setPtr="$2"
	if [[ "$objectName" == "" ]]; then
		objectName="$RANDOM$RANDOM$RANDOM$RANDOM"
	fi
	if [[ ! -z "$setPtr" ]]; then
		eval "$setPtr=\"bpp__ScopeDemonstration__${objectName}\""
	fi
}

function bpp__ScopeDemonstration____delete() {
	local objectName="$1" isPtr="$2"
	local objectAddress="bpp__ScopeDemonstration__${objectName}"
	if [[ "$isPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
}

function bpp__ScopeDemonstration____copy() {
	local copyFrom="$1" copyTo="$2" copyFromIsPtr="$3" copyToIsPtr="$4"
}

function bpp__ScopeDemonstration__toPrimitive() {
	echo "ScopeDemonstration Instance"
}

function bpp__ScopeDemonstration__call() {
	bpp__Person____new "mike2"
	bpp__Person____constructor__Person "mike2" 0 "bpp__Person__mike"
	bpp__Person____destructor "mike2" 0
}

bpp__ScopeDemonstration____new "demo"
bpp__ScopeDemonstration__call

