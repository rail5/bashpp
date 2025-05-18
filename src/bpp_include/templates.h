/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_TEMPLATES_H_
#define SRC_BPP_INCLUDE_TEMPLATES_H_

static const char* bpp_supershell_function = R"EOF(function bpp____initsupershell() {
	local bpp____supershellDirectory="/dev/shm/"
	if [[ ! -d "${bpp____supershellDirectory}" ]]; then
		bpp____supershellDirectory="${TMPDIR:-/tmp/}"
	fi
	local bpp____supershelltempfile="$(mktemp "${bpp____supershellDirectory}/XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")"
	eval "exec {bpp____supershellFD__$BASHPID}<>\"$bpp____supershelltempfile\""
	rm "$bpp____supershelltempfile"
}
function bpp____supershell() {
	local __outputVar="$1" __command="$2" __supershellFD="bpp____supershellFD__$BASHPID" __temporaryStorage=""
	if [[ -z "${!__supershellFD}" ]]; then
		bpp____initsupershell
	else
		__temporaryStorage=$(< "/dev/fd/${!__supershellFD}")
	fi
	$__command 1>"/dev/fd/${!__supershellFD}" 2>/dev/null
	eval "$__outputVar=\$(< "/dev/fd/${!__supershellFD}")"
	echo "${__temporaryStorage}">"/dev/fd/${!__supershellFD}"
}
)EOF";

static const char* bpp_vtable_lookup = R"EOF(function bpp____vTable__lookup() {
	local __objectAddress="$1" __method="$2" __outputVar="$3"
	([[ -z "${__objectAddress}" ]] || [[ -z "${__method}" ]] || [[ -z "${__outputVar}" ]]) && >&2 echo "Bash++: Error: Invalid vTable lookup" && exit 1
	while : ; do
		if ! eval "declare -p \"${__objectAddress}\"" &>/dev/null; then
			break
		fi
		[[ -z "${!__objectAddress}" ]] && break
		__objectAddress="${!__objectAddress}"
	done
	local __vTable="${__objectAddress}____vPointer"
	if ! eval "declare -p \"${__vTable}\"" &>/dev/null; then
		return 1
	fi
	local __result="${!__vTable}[\"${__method}\"]"
	[[ -z "${!__result}" ]] && >&2 echo "Bash++: Error: Method '${__method}' not found in vTable for object '${__objectAddress}'" && return 1
	eval "${__outputVar}=\$__result"
}
)EOF";

static const char* bpp_dynamic_cast = R"EOF(function bpp____dynamic__cast() {
	local __objectAddress="$1" __type="$2" __outputVar="$3"
	([[ -z "${__objectAddress}" ]] || [[ -z "${__type}" ]] || [[ -z "${__outputVar}" ]]) && >&2 echo "Bash++: Error: Invalid dynamic_cast" && exit 1
	eval "${__outputVar}=0"
	while [[ ! -z "${!__objectAddress}" ]] 2>/dev/null; do
		__objectAddress="${!__objectAddress}"
	done
	local __vTable="${__objectAddress}____vPointer"
	while [[ ! -z "${!__vTable}" ]] 2>/dev/null; do
		[[ "${!__vTable}" == "bpp__${__type}____vTable" ]] && eval "${__outputVar}=\"${__objectAddress}\"" && return 0
		__vTable="${!__vTable}[\"__parent__\"]"
	done
	return 1
}
)EOF";

static const char* template_new_function = R"EOF(function bpp__%CLASS%____new() {
	local __objectAddress="$1"
	if [[ "${__objectAddress}" == "" ]]; then
		while : ; do
			__objectAddress="bpp__%CLASS%__$RANDOM$RANDOM$RANDOM$RANDOM"
			local __unusedVar="${__objectAddress}____vPointer"
			[[ -z "${!__unusedVar+x}" ]] && break
		done
	fi
	eval "${__objectAddress}____vPointer=bpp__%CLASS%____vTable"
%ASSIGNMENTS%
	echo "${__objectAddress}"
}
)EOF";

static const char* template_copy_function = R"EOF(function bpp__%CLASS%____copy() {
	local __copyFromAddress="$1" __copyToAddress="$2"
	while : ; do
		if ! eval "declare -p \"${__copyFromAddress}\"" &>/dev/null; then
			break
		fi
		[[ -z "${!__copyFromAddress}" ]] && break
		__copyFromAddress="${!__copyFromAddress}"
	done
	while : ; do
		if ! eval "declare -p \"${__copyToAddress}\"" &>/dev/null; then
			break
		fi
		[[ -z "${!__copyToAddress}" ]] && break
		__copyToAddress="${!__copyToAddress}"
	done
	local __copyFromVPointer="${__copyFromAddress}____vPointer" __copyToVPointer="${__copyToAddress}____vPointer"
	if [[ "${__copyFromAddress}" == "0" ]] || [[ -z "${!__copyFromVPointer}" ]]; then
		>&2 echo "Bash++: Error: %CLASS%: Attempted to copy from null object"
		return
	fi
	if [[ "${__copyToAddress}" == "0" ]] || [[ -z "${!__copyToVPointer}" ]]; then
		>&2 echo "Bash++: Error: %CLASS%: Attempted to copy to null object"
		return
	fi
%COPIES%
	eval "${__copyToVPointer}=${!__copyFromVPointer}"
}
)EOF";

static const char* template_method = R"EOF(function bpp__%CLASS%__%SIGNATURE%() {
	local __objectAddress="$1"
	shift 1
	%PARAMS%
	while : ; do
		if ! eval "declare -p \"${__objectAddress}\"" &>/dev/null; then
			break
		fi
		[[ -z "${!__objectAddress}" ]] && break
		__objectAddress="${!__objectAddress}"
	done
	local __vPointer="${__objectAddress}____vPointer"
	if [[ "${__objectAddress}" == "0" ]] || [[ -z "${!__vPointer}" ]]; then
		>&2 echo "Bash++: Error: Attempted to call @%CLASS%.%SIGNATURE% on null object"
		return
	fi
%METHODBODY%
}
)EOF";

#endif // SRC_BPP_INCLUDE_TEMPLATES_H_
