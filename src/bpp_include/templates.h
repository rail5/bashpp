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
		__temporaryStorage=$(< "/proc/self/fd/${!__supershellFD}")
	fi
	$__command 1>"/proc/self/fd/${!__supershellFD}" 2>/dev/null
	eval "$__outputVar=\$(< "/proc/self/fd/${!__supershellFD}")"
	echo "${__temporaryStorage}">"/proc/self/fd/${!__supershellFD}"
}
)EOF";

const char* template_new_function = R"EOF(function bpp__%CLASS%____new() {
	local __objectName="$1" __setPtr="$2"
	if [[ "${__objectName}" == "" ]]; then
		while : ; do
			__objectName="$RANDOM$RANDOM$RANDOM$RANDOM"
			local __unusedVar="bpp__%CLASS%__${__objectName}____inUse"
			[[ -z "${!__unusedVar+x}" ]] && break
		done
	fi
	local __objectAddress="bpp__%CLASS%__${__objectName}"
	eval "${__objectAddress}____inUse=1"
%ASSIGNMENTS%
	if [[ ! -z "${__setPtr}" ]]; then
		eval ${__setPtr}=\${__objectAddress}
	fi
}
)EOF";

const char* template_delete_function = R"EOF(function bpp__%CLASS%____delete() {
	local __objectName="$1" __objectIsPtr="$2"
	local __objectAddress="${__objectName}"
	if [[ "${__objectIsPtr}" -ne 1 ]]; then
		__objectAddress="bpp__%CLASS%__${__objectName}"
	fi
	local __inUseVar="${__objectAddress}____inUse"
	if [[ "${__objectAddress}" == "0" ]] || [[ "${!__inUseVar}" -ne 1 ]]; then
		>&2 echo "Bash++: Error: Attempted to delete null object"
		return
	fi
%DELETIONS%
	unset ${__objectAddress}____inUse
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
	local __copyFromInUseVar="${__copyFromAddress}____inUse" __copyToInUseVar="${__copyToAddress}____inUse"
	if [[ "${__copyFromAddress}" == "0" ]] || [[ "${!__copyFromInUseVar}" -ne 1 ]]; then
		>&2 echo "Bash++: Error: Attempted to copy from null object"
		return
	fi
	if [[ "${__copyToAddress}" == "0" ]] || [[ "${!__copyToInUseVar}" -ne 1 ]]; then
		>&2 echo "Bash++: Error: Attempted to copy to null object"
		return
	fi
%COPIES%
	eval "${__copyToAddress}____inUse=1"
}
)EOF";

const char* template_method = R"EOF(function bpp__%CLASS%__%SIGNATURE%() {
	local __objectName="$1" __objectIsPtr="$2"
	shift 2
	local __objectAddress="${__objectName}" %PARAMS%
	if [[ "${__objectIsPtr}" -ne 1 ]]; then
		__objectAddress="bpp__%CLASS%__${__objectName}"
	fi
	local __inUseVar="${__objectAddress}____inUse"
	if [[ "${__objectAddress}" == "0" ]] || [[ "${!__inUseVar}" -ne 1 ]]; then
		>&2 echo "Bash++: Error: Attempted to call method on null object"
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
	local __inUseVar="${__objectAddress}____inUse"
	if [[ "${__objectAddress}" == "0" ]] || [[ "${!__inUseVar}" -ne 1 ]]; then
		>&2 echo "Bash++: Error: Attempted to construct null object"
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
	local __inUseVar="${__objectAddress}____inUse"
	if [[ "${__objectAddress}" == "0" ]] || [[ "${!__inUseVar}" -ne 1 ]]; then
		>&2 echo "Bash++: Error: Attempted to destruct null object"
		return
	fi
%DESTRUCTORBODY%
}
)EOF";

#endif // SRC_BPP_INCLUDE_TEMPLATES_H_
