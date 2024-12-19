#!/usr/bin/env bash

function bpp__Object____new() {
	local objectName="$1" setPtr="$2"
	if [[ "$objectName" == "" ]]; then
		while : ; do
			objectName="$RANDOM$RANDOM$RANDOM$RANDOM"
			local exemplaryDataMember="bpp__Object__${objectName}__data"
			[[ -z ${!exemplaryDataMember+x} ]] && break
		done
	fi
	eval "bpp__Object__${objectName}__data=\"default value for primitive data member\""
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
	eval "unset ${objectAddress}__data"
}

function bpp__Object____getAddress() {
	local objectName="$1"
	echo "bpp__Object__${objectName}"
}

function bpp__Object____copy() {
	local copyFrom="$1" copyTo="$2" copyFromIsPtr="$3" copyToIsPtr="$4"
	local copyFrom__data="bpp__Object__${copyFrom}__data" copyTo__data="bpp__Object__${copyTo}__data"
	if [[ "$copyFromIsPtr" -eq 1 ]]; then
		copyFrom__data="${copyFrom}__data"
	fi
	if [[ "$copyToIsPtr" -eq 1 ]]; then
		copyTo__data="${copyTo}__data"
	fi
	eval "$copyTo__data=\"${!copyFrom__data}\""
}

function bpp__Object__toPrimitive() {
	local objectName="$1"
	echo "Object Instance"
}

bpp__Object____new "testObject1"
bpp__ptr__Object__testObject2="nullptr"
bpp__Object____new "" "bpp__ptr__Object__testObject3"
bpp__ptr__Object__testObject3Copy="$bpp__ptr__Object__testObject3"
bpp__Object____copy "$bpp__ptr__Object__testObject3" "testObject4" 1 0
bpp__ptr__Object__testObject5="$(bpp__Object____getAddress "testObject4")"

echo "$bpp__Object__testObject1__data"
# Implicit pointer dereference
bpp__ptr__Object__testObject2__data="${bpp__ptr__Object__testObject2}__data"
echo "${!bpp__ptr__Object__testObject2__data}"
unset bpp__ptr__Object__testObject2__data

# Implicit pointer dereference
eval "${bpp__ptr__Object__testObject3}__data=\"new value for object number 3\""
# Implicit pointer dereference
bpp__ptr__Object__testObject3__data="${bpp__ptr__Object__testObject3}__data"
echo "${!bpp__ptr__Object__testObject3__data}"
unset bpp__ptr__Object__testObject3__data
# Implicit pointer dereference
bpp__ptr__Object__testObject3Copy__data="${bpp__ptr__Object__testObject3Copy}__data"
echo "${!bpp__ptr__Object__testObject3Copy__data}"
unset bpp__ptr__Object__testObject3__data

echo "$bpp__Object__testObject4__data"
# Implicit pointer dereference
bpp__ptr__Object__testObject5__data="${bpp__ptr__Object__testObject5}__data"
echo "${!bpp__ptr__Object__testObject5__data}"
unset bpp__ptr__Object__testObject5__data

bpp__Object____delete "testObject1" 0
bpp__Object____delete "$bpp__ptr__Object__testObject2" 1
bpp__Object____delete "$bpp__ptr__Object__testObject3" 1
bpp__Object____delete "$bpp__ptr__Object__testObject3Copy" 1
bpp__Object____delete "testObject4" 0
bpp__Object____delete "$bpp__ptr__Object__testObject5" 1
