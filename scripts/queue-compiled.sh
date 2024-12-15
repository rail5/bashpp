#!/usr/bin/env bash

function bpp_Node__new() {
	local objectName="$1"
	eval "bpp_Node_${objectName}_data=''"
	eval "bpp_Node_${objectName}_next='nullptr'"
}

function bpp_Queue__new() {
	local objectName="$1"
	eval "bpp_Queue_${objectName}_queueHead='nullptr'"
	eval "bpp_Queue_${objectName}_queueTail='nullptr'"
	eval "bpp_Queue_${objectName}_size=0"
}

function bpp_Queue_enqueue() {
	if [[ $# -ne 2 ]]; then
		echo "Bash++: Invalid number of arguments passed to @Queue.enqueue"
		echo "(Expected 1, got $(($# - 1)))"
		exit 1
	fi
	local objectName="$1" data="$2"
	local queueHeadVar="bpp_Queue_${objectName}_queueHead"
	local queueTailVar="bpp_Queue_${objectName}_queueTail"
	local sizeVar="bpp_Queue_${objectName}_size"
	local queueHead="${!queueHeadVar}"
	local queueTail="${!queueTailVar}"
	local size="${!sizeVar}"

	if [[ "$size" -eq 0 ]]; then
		local newNode="node${size}"
		bpp_Node__new "$newNode"
		eval "${queueHeadVar}='$newNode'"
		eval "${queueTailVar}='$newNode'"
		eval "bpp_Node_${newNode}_data='$data'"
	else
		local newNode="node${size}"
		bpp_Node__new "$newNode"
		eval "bpp_Node_${queueTail}_next='$newNode'"
		eval "bpp_Node_${newNode}_data='$data'"
		eval "${queueTailVar}='$newNode'"
	fi
	eval "${sizeVar}=$((size + 1))"
}

function bpp_Queue_dequeue() {
	if [[ $# -ne 1 ]]; then
		echo "Bash++: Invalid number of arguments passed to @Queue.dequeue"
		echo "(Expected 0, got $(($# - 1)))"
		exit 1
	fi
	local objectName="$1"
	local queueHeadVar="bpp_Queue_${objectName}_queueHead"
	local sizeVar="bpp_Queue_${objectName}_size"
	local queueHead="${!queueHeadVar}"
	local size="${!sizeVar}"

	if [[ "$size" -eq 0 ]]; then
		return
	fi

	local nextNodeVar="bpp_Node_${queueHead}_next"
	local nextNode="${!nextNodeVar}"
	local dataVar="bpp_Node_${queueHead}_data"
	local data="${!dataVar}"

	eval "unset bpp_Node_${queueHead}_data"
	eval "unset bpp_Node_${queueHead}_next"
	eval "${queueHeadVar}='$nextNode'"
	eval "${sizeVar}=$((size - 1))"
	echo "$data"
}

function bpp_Queue_isEmpty() {
	if [[ $# -ne 1 ]]; then
		echo "Bash++: Invalid number of arguments passed to @Queue.isEmpty"
		echo "(Expected 0, got $(($# - 1)))"
		exit 1
	fi
	local objectName="$1"
	local sizeVar="bpp_Queue_${objectName}_size"
	local size="${!sizeVar}"

	if [[ "$size" -eq 0 ]]; then
		echo "true"
	else
		echo "false"
	fi
}

bpp_Queue__new "testQueue"
bpp_Queue_enqueue "testQueue" "a"
bpp_Queue_enqueue "testQueue" "b"
bpp_Queue_enqueue "testQueue" "c"

while [[ $(bpp_Queue_isEmpty "testQueue") != "true" ]]; do
	bpp_Queue_dequeue "testQueue"
done

