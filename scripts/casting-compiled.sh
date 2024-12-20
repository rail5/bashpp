#!/usr/bin/env bash

function bpp__Object____new() {
	local objectName="$1" setPtr="$2"
	if [[ "$objectName" == "" ]]; then
		while : ; do
			objectName="$RANDOM$RANDOM$RANDOM$RANDOM"
			local exemplaryDataMember="bpp__Object__${objectName}__innerDataMember"
			[[ -z ${!exemplaryDataMember+x} ]] && break
		done
	fi
	eval "bpp__Object__${objectName}__innerDataMember=\"default value\""
	if [[ ! -z "$setPtr" ]]; then
		eval "$setPtr=\"bpp__Object__${objectName}\""
	fi
}

function bpp__Object____delete() {
	local objectName="$1" isPtr="$2"
	local objectAddress="bpp__Object__${objectName}"
	if [[ "$isPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	eval "unset ${objectAddress}__innerDataMember"
}

function bpp__Object____copy() {
	local copyFrom="$1" copyTo="$2" copyFromIsPtr="$3" copyToIsPtr="$4"
	local copyFrom__innerDataMember="bpp__Object__${copyFrom}__innerDataMember" copyTo__innerDataMember="bpp__Object__${copyTo}__innerDataMember"
	if [[ "$copyFromIsPtr" -eq 1 ]]; then
		copyFrom__innerDataMember="${copyFrom}__innerDataMember"
	fi
	if [[ "$copyToIsPtr" -eq 1 ]]; then
		copyTo__innerDataMember="${copyTo}__innerDataMember"
	fi
	eval "$copyTo__innerDataMember=\"${!copyFrom__innerDataMember}\""
}

function bpp__Object__toPrimitive() {
	echo "Object Instance"
}

function bpp__Object__display() {
	if [[ $# -ne 2 ]]; then
		echo "Bash++: Invalid number of arguments passed to @Object.display"
		echo "(Expected 0, got $(($# - 2)))"
	fi
	local objectName="$1" objectIsPtr="$2"
	local objectAddress="bpp__Object__${objectName}"
	if [[ "$objectIsPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__innerDataMember="${objectAddress}__innerDataMember"
	echo "${!this__innerDataMember}"
}

function bpp__Object__update() {
	if [[ $# -ne 3 ]]; then
		echo "Bash++: Invalid number of arguments passed to @Object.update"
		echo "(Expected 1, got $(($# - 2)))"
	fi
	local objectName="$1" objectIsPtr="$2" newValue="$3"
	local objectAddress="bpp__Object__${objectName}"
	if [[ "$objectIsPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__innerDataMember="${objectAddress}__innerDataMember"
	eval "${this__innerDataMember}=\"$newValue\""
}

function bpp__ObjectWithCasting____new() {
	local objectName="$1" setPtr="$2"
	if [[ "$objectName" == "" ]]; then
		while : ; do
			objectName="$RANDOM$RANDOM$RANDOM$RANDOM"
			local exemplaryDataMember="bpp__ObjectWithCasting__${objectName}__outerDataMember"
			[[ -z ${!exemplaryDataMember+x} ]] && break
		done
	fi
	# A non-primitive object data member is declared to be part of the class
	# We will store, within *this* object, a *pointer* to that non-primitive member
	# Declaring an OBJECT to be a member (as opposed merely to an object POINTER),
	# Means that when this object goes out of scope or is @delete'd,
	# It will also delete the "object" it contains
	# Whereas, if the data member was declared as a pointer,
	# That memory would not necessarily automatically be freed.
	# Syntactically, this makes little difference,
	# As pointers are IMPLICITLY dereferenced automatically. So a programmer using Bash++
	# Can access the member functions and data members of a pointer using exactly
	# The same syntax as if she were referring to the object directly
	# Ie, @Object* someObjectPtr ---> @someObjectPtr.runFunction
	# The same as @Object someObject ---> @someObject.runFunction
	bpp__Object____new "" "bpp__ObjectWithCasting__${objectName}__outerDataMember"
	if [[ ! -z "$setPtr" ]]; then
		eval "$setPtr=\"bpp__ObjectWithCasting__${objectName}\""
	fi
}

function bpp__ObjectWithCasting____delete() {
	local objectName="$1" isPtr="$2"
	local objectAddress="bpp__ObjectWithCasting__${objectName}"
	if [[ "$isPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	# Automatically free inner data member memory
	local this__outerDataMember="${objectAddress}__outerDataMember"
	bpp__Object____delete "${!this__outerDataMember}" 1
	eval "unset ${objectAddress}__outerDataMember"
}

function bpp__ObjectWithCasting____copy() {
	local copyFrom="$1" copyTo="$2" copyFromIsPtr="$3" copyToIsPtr="$4"
	local copyFrom__outerDataMember="bpp__ObjectWithCasting__${copyFrom}__outerDataMember" copyTo__data="bpp__ObjectWithCasting__${copyTo}__outerDataMember"
	if [[ "$copyFromIsPtr" -eq 1 ]]; then
		copyFrom__outerDataMember="${copyFrom}__outerDataMember"
	fi
	if [[ "$copyToIsPtr" -eq 1 ]]; then
		copyTo__outerDataMember="${copyTo}__outerDataMember"
	fi
	# Run non-primitive copy functions on data members
	bpp__Object____copy "${!copyFrom__outerDataMember}" "${!copyTo__outerDataMember}" 1 1
}

function bpp__ObjectWithCasting__toPrimitive() {
	# User defined overriden toPrimitive method
	local objectName="$1" objectIsPtr="$2"
	local objectAddress="bpp__ObjectWithCasting__${objectName}"
	if [[ "$objectIsPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__outerDataMember="${objectAddress}__outerDataMember"
	bpp__Object__display "${!this__outerDataMember}" 1
}

function bpp__ObjectWithCasting__update__primitive() {
	if [[ $# -ne 3 ]]; then
		echo "Bash++: Invalid number of arguments passed to @ObjectWithCasting.update"
		echo "(Expected 1, got $(($# - 2)))"
	fi
	local objectName="$1" objectIsPtr="$2" newValue="$3"
	local objectAddress="bpp__ObjectWithCasting__${objectName}"
	if [[ "$objectIsPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__outerDataMember="${objectAddress}__outerDataMember"
	bpp__Object__update "${!this__outerDataMember}" 1 "$newValue"
}

function bpp__ObjectWithCasting__update__Object() {
	if [[ $# -ne 3 ]]; then
		echo "Bash++: Invalid number of arguments passed to @ObjectWithCasting.update"
		echo "(Expected 1, got $(($# - 2)))"
	fi
	local objectName="$1" objectIsPtr="$2" newValue="$3"
	local objectAddress="bpp__ObjectWithCasting__${objectName}"
	if [[ "$objectIsPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__outerDataMember="${objectAddress}__outerDataMember"
	bpp__Object____delete "${!this__outerDataMember}" 1
	bpp__Object____copy "${!newValue}" "${!this__outerDataMember}" 1 1
}

bpp__Object____new "testObject1"
bpp__Object__update "testObject1" 0 "Test Object #1"
# Implicit call to toPrimitive
echo "$(bpp__Object__toPrimitive "testObject1" 0)"

bpp__ObjectWithCasting____new "testObject2"
bpp__ObjectWithCasting__update__primitive "testObject2" 0 "Test Object #2"
# Implicit call to toPrimitive
echo "$(bpp__ObjectWithCasting__toPrimitive "testObject2" 0)"

