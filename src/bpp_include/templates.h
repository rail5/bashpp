/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_TEMPLATES_H_
#define SRC_BPP_INCLUDE_TEMPLATES_H_

const char* bpp_supershell_function = R"EOF(function bpp____initsupershell() {
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

const char* bpp_vtable_lookup = R"EOF(function bpp____vTable__lookup() {
	local __objectAddress="$1" __method="$2" __outputVar="$3"
	([[ -z "${__objectAddress}" ]] || [[ -z "${__method}" ]] || [[ -z "${__outputVar}" ]]) && >&2 echo "Bash++: Error: Invalid vTable lookup" && exit 1
	while [[ ! -z "${!__objectAddress}" ]]; do
		__objectAddress="${!__objectAddress}"
	done
	local __vTable="${__objectAddress}____vPointer"
	[[ -z "${!__vTable}" ]] && >&2 echo "Bash++:: Error: vTable not found for object '${__objectAddress}'" && exit 1
	local __result="${!__vTable}[\"${__method}\"]"
	[[ -z "${!__result}" ]] && >&2 echo "Bash++: Error: Method '${__method}' not found in vTable for object '${__objectAddress}'" && exit 1
	eval "${__outputVar}=\$__result"
}
)EOF";

const char* template_new_function = R"EOF(function bpp__%CLASS%____new() {
	local __objectName="$1"
	if [[ "${__objectName}" == "" ]]; then
		while : ; do
			__objectName="$RANDOM$RANDOM$RANDOM$RANDOM"
			local __unusedVar="bpp__%CLASS%__${__objectName}____vPointer"
			[[ -z "${!__unusedVar+x}" ]] && break
		done
	fi
	local __objectAddress="bpp__%CLASS%__${__objectName}"
	eval "${__objectAddress}____vPointer=bpp__%CLASS%____vTable"
%ASSIGNMENTS%
	echo "${__objectAddress}"
}
)EOF";

const char* template_delete_function = R"EOF(function bpp__%CLASS%____delete() {
	local __objectName="$1" __objectIsPtr="$2"
	local __objectAddress="${__objectName}"
	if [[ "${__objectIsPtr}" -ne 1 ]]; then
		__objectAddress="bpp__%CLASS%__${__objectName}"
	fi
	local __vPointer="${__objectAddress}____vPointer"
	if [[ "${__objectAddress}" == "0" ]] || [[ -z "${!__vPointer}" ]]; then
		>&2 echo "Bash++: Error: %CLASS%: Attempted to delete null object"
		return
	fi
%DELETIONS%
	unset ${__objectAddress}____vPointer
}
)EOF";

const char* template_copy_function = R"EOF(function bpp__%CLASS%____copy() {
	local __copyFrom="$1" __copyTo="$2" __copyFromIsPtr="$3" __copyToIsPtr="$4"
	local __copyFromAddress="${__copyFrom}" __copyToAddress="${__copyTo}"
	if [[ "${__copyFromIsPtr}" -ne 1 ]]; then
		__copyFromAddress="bpp__%CLASS%__${__copyFrom}"
	fi
	if [[ "${__copyToIsPtr}" -ne 1 ]]; then
		__copyToAddress="bpp__%CLASS%__${__copyTo}"
	fi
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

const char* template_method = R"EOF(function bpp__%CLASS%__%SIGNATURE%() {
	local __objectName="$1" __objectIsPtr="$2"
	shift 2
	local __objectAddress="${__objectName}" %PARAMS%
	if [[ "${__objectIsPtr}" -ne 1 ]]; then
		__objectAddress="bpp__%CLASS%__${__objectName}"
	fi
	local __vPointer="${__objectAddress}____vPointer"
	if [[ "${__objectAddress}" == "0" ]] || [[ -z "${!__vPointer}" ]]; then
		>&2 echo "Bash++: Error: Attempted to call @%CLASS%.%SIGNATURE% on null object"
		return
	fi
%METHODBODY%
}
)EOF";

const char* template_constructor = R"EOF(function bpp__%CLASS%____constructor() {
	local __objectName="$1" __objectIsPtr="$2"
	local __objectAddress="${__objectName}"
	if [[ "${__objectIsPtr}" -ne 1 ]]; then
		__objectAddress="bpp__%CLASS%__${__objectName}"
	fi
	local __vPointer="${__objectAddress}____vPointer"
	if [[ "${__objectAddress}" == "0" ]] || [[ -z "${!__vPointer}" ]]; then
		>&2 echo "Bash++: Error: %CLASS%: Attempted to construct null object"
		return
	fi
%CONSTRUCTORBODY%
}
)EOF";

const char* template_destructor = R"EOF(function bpp__%CLASS%____destructor() {
	local __objectName="$1" __objectIsPtr="$2"
	local __objectAddress="${__objectName}"
	if [[ "${__objectIsPtr}" -ne 1 ]]; then
		__objectAddress="bpp__%CLASS%__${__objectName}"
	fi
	local __vPointer="${__objectAddress}____vPointer"
	if [[ "${__objectAddress}" == "0" ]] || [[ -z "${!__vPointer}" ]]; then
		>&2 echo "Bash++: Error: %CLASS%: Attempted to destruct null object"
		return
	fi
%DESTRUCTORBODY%
}
)EOF";

#endif // SRC_BPP_INCLUDE_TEMPLATES_H_
