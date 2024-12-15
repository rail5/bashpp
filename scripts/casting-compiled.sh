#!/usr/bin/env bash

function bpp_Object__new() {
	local objectName="$1"
	eval "bpp_Object_${objectName}_innerDataMember='default value'"
}

function bpp_Object__copy() {
	local copyFrom="$1" copyTo="$2"
	local copyFrom_innerDataMemberVar="bpp_Object_${copyFrom}_innerDataMember"
	local copyTo_innerDataMemberVar="bpp_Object_${copyTo}_innerDataMember"
	local copyTo_innerDataMember="${!copyFrom_innerDataMemberVar}"
	eval "$copyTo_innerDataMemberVar='$copyTo_innerDataMember'"
}

function bpp_Object__delete() {
	local objectName="$1"
	eval "unset bpp_Object_${objectName}_innerDataMember"
}

function bpp_Object_toPrimitive() {
	echo "Object Instance"
}

function bpp_Object_display() {
	if [[ $# -ne 1 ]]; then
		echo "Bash++: Invalid number of arguments passed to @Object.display"
		echo "(Expected 0, got $(($# - 1)))"
		exit 1
	fi
	local objectName="$1"
	local innerDataMemberVar="bpp_Object_${objectName}_innerDataMember"
	
	echo "${!innerDataMemberVar}"
}

function bpp_Object_update() {
	if [[ $# -ne 2 ]]; then
		echo "Bash++: Invalid number of arguments passed to @Object.update"
		echo "(Expected 1, got $(($# - 1)))"
		exit 1
	fi
	local objectName="$1" newValue="$2"
	local innerDataMemberVar="bpp_Object_${objectName}_innerDataMember"
	
	eval "$innerDataMemberVar='$newValue'"
}

function bpp_ObjectWithCasting__new() {
	local objectName="$1"
	bpp_Object__new "bpp_ObjectWithCasting_${objectName}_outerDataMember"
}

function bpp_ObjectWithCasting_toPrimitive() {
	if [[ $# -ne 1 ]]; then
		echo "Bash++: Invalid number of arguments passed to @ObjectWithCasting.toPrimitive"
		echo "(Expected 0, got $(($# - 1)))"
		exit 1
	fi
	local objectName="$1"

	bpp_Object_display "bpp_ObjectWithCasting_${objectName}_outerDataMember"
}

function bpp_ObjectWithCasting_update__primitive() {
	if [[ $# -ne 2 ]]; then
		echo "Bash++: Invalid number of arguments passed to @ObjectWithCasting.update {primitive}"
		echo "(Expected 1, got $(($# - 1)))"
		exit 1
	fi
	local objectName="$1" newValue="$2"

	bpp_Object_update "bpp_ObjectWithCasting_${objectName}_outerDataMember" "$newValue"
}

function bpp_ObjectWithCasting_update__Object() {
	if [[ $# -ne 2 ]]; then
		echo "Bash++: Invalid number of arguments passed to @ObjectWithCasting.update {Object}"
		echo "(Expected 1, got $(($# - 1)))"
		exit 1
	fi
	local objectName="$1" newValueObjectName="$2"
	
	bpp_Object__delete "bpp_ObjectWithCasting_${objectName}_outerDataMember"
	bpp_object__copy "$newValueObjectName" "bpp_ObjectWithCasting_${objectName}_outerDataMember"
}

bpp_Object__new "testObject1"
bpp_Object_update "testObject1" "Test Object #1"
echo "$(bpp_Object_toPrimitive "testObject1")"

bpp_ObjectWithCasting__new "testObject2"
bpp_ObjectWithCasting_update__primitive "testObject2" "Test Object #2"
echo "$(bpp_ObjectWithCasting_toPrimitive "testObject2")"
