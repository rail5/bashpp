#!/usr/bin/env bash

function bpp__Node____new() {
	local objectName="$1" setPtr="$2"
	if [[ "$objectName" == "" ]]; then
		while : ; do
			objectName="$RANDOM$RANDOM$RANDOM$RANDOM"
			local exemplaryDataMember="bpp__Node__${objectName}__data"
			[[ -z "${!exemplaryDataMember+x}" ]] && break
		done
	fi
	eval "bpp__Node__${objectName}__data=''"
	eval "bpp__Node__${objectName}__next='nullptr'"
	if [[ ! -z "$setPtr" ]]; then
		eval "${setPtr}=\"bpp__Node__${objectName}\""
	fi
}

function bpp__Node____delete() {
	local objectName="$1" objectIsPtr="$2"
	local objectAddress="bpp__Node__${objectName}"
	if [[ "$objectIsPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	eval "unset ${objectAddress}__data"
	eval "unset ${objectAddress}__next"
}

function bpp__Node____copy() {
	local copyFrom="$1" copyTo="$2" copyFromIsPtr="$3" copyToIsPtr="$4"
	local copyFromAddress="bpp__Node__${copyFrom}" copyToAddress="bpp__Node__${copyTo}"
	if [[ "$copyFromIsPtr" -eq 1 ]]; then
		copyFromAddress="${copyFrom}"
	fi
	if [[ "$copyToIsPtr" -eq 1 ]]; then
		copyToAddress="${copyTo}"
	fi
	local copyFrom__data="${copyFromAddress}__data" copyFrom__next="${copyFromAddress}__next"
	eval "${copyToAddress}__data=\"${!copyFrom__data}\""
	eval "${copyToAddress}__next=\"${!copyFrom__next}\""
}

function bpp__Node__toPrimitive() {
	echo "Node Instance"
}

function bpp__Queue____new() {
	local objectName="$1" setPtr="$2"
	if [[ "$objectName" == "" ]]; then
		while : ; do
			objectName="$RANDOM$RANDOM$RANDOM$RANDOM"
			local exemplaryDataMember="bpp__Queue__${objectName}__queueHead"
			[[ -z "${!exemplaryDataMember+x}" ]] && break
		done
	fi
	eval "bpp__Queue__${objectName}__queueHead='nullptr'"
	eval "bpp__Queue__${objectName}__queueTail='nullptr'"
	eval "bpp__Queue__${objectName}__size='0'"
	if [[ ! -z "$setPtr" ]]; then
		eval "${setPtr}=\"bpp__Queue__${objectName}\""
	fi
}

function bpp__Queue____delete() {
	local objectName="$1" objectIsPtr="$2"
	local objectAddress="bpp__Queue__${objectName}"
	if [[ "$objectIsPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	eval "unset ${objectAddress}__queueHead"
	eval "unset ${objectAddress}__queueTail"
	eval "unset ${objectAddress}__size"
}

function bpp__Queue____copy() {
	local copyFrom="$1" copyTo="$2" copyFromIsPtr="$3" copyToIsPtr="$4"
	local copyFromAddress="bpp__Queue__${copyFrom}" copyToAddress="bpp__Queue__${copyTo}"
	if [[ "$copyFromIsPtr" -eq 1 ]]; then
		copyFromAddress="${copyFrom}"
	fi
	if [[ "$copyToIsPtr" -eq 1 ]]; then
		copyToAddress="${copyTo}"
	fi
	local copyFrom__queueHead="${copyFromAddress}__queueHead" copyFrom__queueTail="${copyFromAddress}__queueTail" copyFrom__size="${copyFromAddress}__size"
	eval "${copyToAddress}__queueHead=\"${!copyFrom__queueHead}\""
	eval "${copyToAddress}__queueTail=\"${!copyFrom__queueTail}\""
	eval "${copyToAddress}__size=\"${!copyFrom__size}\""
}

function bpp__Queue__toPrimitive() {
	echo "Queue Instance"
}

# User-defined Queue methods
function bpp__Queue__enqueue() {
	if [[ $# -ne 3 ]]; then
		echo "Bash++: Invalid number of arguments passed to @Queue.enqueue"
		echo "(Expected 1, got $(($# - 2)))"
	fi
	local objectName="$1" objectIsPtr="$2" data="$3"
	local objectAddress="bpp__Queue__${objectName}"
	if [[ "$objectIsPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__size="${objectAddress}__size" this__queueHead="${objectAddress}__queueHead" this__queueTail="${objectAddress}__queueTail"
	if [[ "${!this__size}" -eq 0 ]]; then
		bpp__Node____new "" "$this__queueHead"
		# Implicit pointer dereference
		local this__queueHead__data="${!this__queueHead}__data"
		eval "${this__queueHead__data}=\"$data\""
		unset this__queueHead__data
		eval "${this__queueTail}=\"${!this__queueHead}\""
	else
		bpp__Node____new "" "bpp__ptr__Node__anotherNode" # Locally declared global-scoped pointer, must erase pointer before method exit
		# Implicit pointer dereference
		local bpp__ptr__Node__anotherNode__data="${bpp__ptr__Node__anotherNode}__data"
		eval "${bpp__ptr__Node__anotherNode__data}=\"$data\""
		unset bpp__ptr__Node__anotherNode__data
		# Implicit pointer dereference
		local this__queueTail__next="${!this__queueTail}__next"
		eval "${this__queueTail__next}=\"${bpp__ptr__Node__anotherNode}\""
		unset this__queueTail__next
		# Implicit pointer dereference
		local this__queueTail__next="${!this__queueTail}__next"
		eval "${this__queueTail}=\"${!this__queueTail__next}\""
		unset this__queueTail__next
		# Erase locally declared pointer
		unset bpp__ptr__Node__anotherNode
	fi
	eval "${this__size}=\"$(($this__size + 1))\""
}

function bpp__Queue__dequeue() {
	if [[ $# -ne 2 ]]; then
		echo "Bash++: Invalid number of arguments passed to @Queue.enqueue"
		echo "(Expected 0, got $(($# - 2)))"
	fi
	local objectName="$1" objectIsPtr="$2"
	local objectAddress="bpp__Queue__${objectName}"
	if [[ "$objectIsPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__size="${objectAddress}__size" this__queueHead="${objectAddress}__queueHead"
	if [[ "${!this__size}" -eq 0 ]]; then
		return
	fi
	# Implicit pointer dereference
	local this__queueHead__next="${!this__queueHead}__next"
	local bpp__ptr__Node__upNext="${!this__queueHead__next}"
	unset this__queueHead__next
	# Implicit pointer dereference
	local this__queueHead__data="${!this__queueHead}__data"
	local data="${!this__queueHead__data}"
	unset this__queueHead__data
	bpp__Node____delete "${!this__queueHead}" 1
	eval "${this__queueHead}=\"${bpp__ptr__Node__upNext}\""
	eval "${this__size}=\"$((${this__size} - 1))\""
	echo "$data"
}

function bpp__Queue__isEmpty() {
	if [[ $# -ne 2 ]]; then
		echo "Bash++: Invalid number of arguments passed to @Queue.enqueue"
		echo "(Expected 0, got $(($# - 2)))"
	fi
	local objectName="$1" objectIsPtr="$2"
	local objectAddress="bpp__Queue__${objectName}"
	if [[ "$objectIsPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__size="${objectAddress}__size"
	if [[ ${!this__size} -eq 0 ]]; then
		echo "true"
	else
		echo "false"
	fi
}

function bpp__Queue__getSize() {
	if [[ $# -ne 2 ]]; then
		echo "Bash++: Invalid number of arguments passed to @Queue.enqueue"
		echo "(Expected 0, got $(($# - 2)))"
	fi
	local objectName="$1" objectIsPtr="$2"
	local objectAddress="bpp__Queue__${objectName}"
	if [[ "$objectIsPtr" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__size="${objectAddress}__size"
	echo "${!this__size}"
}

# Main body
bpp__Queue____new "testQueue"
bpp__Queue__enqueue "testQueue" 0 "a"
bpp__Queue__enqueue "testQueue" 0 "b"
bpp__Queue__enqueue "testQueue" 0 "c"

while [[ "$(bpp__Queue__isEmpty "testQueue" 0)" != "true" ]]; do
	bpp__Queue__dequeue "testQueue" 0
done

