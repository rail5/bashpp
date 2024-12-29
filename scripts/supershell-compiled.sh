#!/usr/bin/env bash

function bpp____initsupershell() {
	local bpp____supershelltempfile="$(mktemp -p /dev/shm/ XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX)"
	eval "exec {bpp____supershellFD__$BASHPID}<>\"$bpp____supershelltempfile\""
	rm "$bpp____supershelltempfile"
}

function bpp____supershell() {
	local __outputVar="$1" __command="$2" __supershellFD="bpp____supershellFD__$BASHPID"
	if [[ -z "${!__supershellFD}" ]]; then
		bpp____initsupershell
	fi
	$__command 1>"/proc/self/fd/${!__supershellFD}" 2>/dev/null
	eval "$__outputVar=\$(< "/proc/self/fd/${!__supershellFD}")"
}

function bpp__MyClass____new() {
	local objectName="$1" setPtr="$2"
	
	if [[ -z "$objectName" ]]; then
		while : ; do
			objectName="$RANDOM$RANDOM$RANDOM$RANDOM"
			local inUse="bpp__MyClass__${objectName}____inUse"
			[[ -z "${!inUse}" ]] && break
		done
	fi
	local objectAddress="bpp__MyClass__${objectName}"
	eval "${objectAddress}____inUse=1"
	eval "${objectAddress}__dataMember=\"a\""
	if [[ ! -z "$setPtr" ]]; then
		eval "${setPtr}=\${objectAddress}"
	fi
}

function bpp__MyClass____delete() {
	local objectName="$1" objectIsPtr="$2"
	local objectAddress="bpp__MyClass__${objectName}"
	if [[ "${objectIsPtr}" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	unset "${objectAddress}__dataMember"
	unset "${objectAddress}____inUse"
}

function bpp__MyClass____copy() {
	local copyFrom="$1" copyTo="$2" copyFromIsPtr="$3" copyToIsPtr="$4"
	local copyFromAddress="bpp__MyClass__${copyFrom}" copyToAddress="bpp__MyClass__${copyTo}"
	if [[ "${copyFromIsPtr}" -eq 1 ]]; then
		copyFromAddress="${copyFrom}"
	fi
	if [[ "${copyToIsPtr}" -eq 1 ]]; then
		copyToAddress="${copyTo}"
	fi
	local copyFrom__dataMember="${copyFromAddress}__dataMember"
	eval "${copyToAddress}__dataMember=\${!copyFrom__dataMember}"
	eval "${copyToAddress}____inUse=1"
}

function bpp__MyClass__toPrimitive() {
	echo "MyClass Instance"
}

function bpp__MyClass__set____primitive() {
	local objectName="$1" objectIsPtr="$2" arg="$3"
	local objectAddress="bpp__MyClass__${objectName}"
	if [[ "${objectIsPtr}" -eq 1 ]]; then
		objectAddress="${objectName}"
	fi
	local this__dataMember="${objectAddress}__dataMember"
	if [[ "${!this__dataMember}" == "$arg" ]]; then
		echo "dataMember is already set to $arg"
	else
		eval "${this__dataMember}=\$arg"
		echo "dataMember was not already set to $arg"
	fi
}

bpp__MyClass____new "myObject"
bpp__MyClass__set____primitive "myObject" 0 "b"
echo "$bpp__MyClass__myObject__dataMember"

function __runfunc() {
	bpp__MyClass__set____primitive "myObject" 0 "c"
}
bpp____supershell "var" __runfunc
unset -f __runfunc
echo "$var"
echo "$bpp__MyClass__myObject__dataMember"
