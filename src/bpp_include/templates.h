/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_TEMPLATES_H_
#define SRC_BPP_INCLUDE_TEMPLATES_H_

/**
 * COMPATIBILITY NOTES:
 *
 * - The $BASHPID internal variable was introduced in Bash 4.0.
 *    This is used by the internal supershell function to ensure forked processes don't overwrite each other's data
 *
 * - The `exec {var}<>` syntax for file descriptors was introduced in Bash 4.1.
 *    This is again used by the internal supershell function
 *
 * - Associative arrays were introduced in Bash 4.0.
 *    This is used by the vTable lookup and dynamic_cast functions to store method pointers and to check types.
 */


[[maybe_unused]] static const char* bpp_supershell_function = R"EOF(function bpp____initsupershell() {
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

[[maybe_unused]] static const char* bpp_repeat = R"EOF(function bpp____repeat() {
	return $1
})EOF";

[[maybe_unused]] static const char* bpp_vtable_lookup = R"EOF(function bpp____vTable__lookup() {
	local __this="$1" __method="$2" __outputVar="$3"
	([[ -z "${__this}" ]] || [[ -z "${__method}" ]] || [[ -z "${__outputVar}" ]]) && >&2 echo "Bash++: Error: Invalid vTable lookup" && exit 1
	while : ; do
		if ! eval "declare -p \"${__this}\"" &>/dev/null; then
			break
		fi
		[[ -z "${!__this}" ]] && break
		__this="${!__this}"
	done
	local __vTable="${__this}____vPointer"
	if ! eval "declare -p \"${__vTable}\"" &>/dev/null; then
		return 1
	fi
	local __result="${!__vTable}[\"${__method}\"]"
	[[ -z "${!__result}" ]] && >&2 echo "Bash++: Error: Method '${__method}' not found in vTable for object '${__this}'" && return 1
	eval "${__outputVar}=\$__result"
}
)EOF";

[[maybe_unused]] static const char* bpp_dynamic_cast = R"EOF(function bpp____dynamic__cast() {
	local __type="$1" __outputVar="$2" __this="$3"
	([[ -z "${__type}" ]] || [[ -z "${__outputVar}" ]]) && >&2 echo "Bash++: Error: Invalid dynamic_cast" && exit 1
	eval "${__outputVar}=0"
	while : ; do
		if ! eval "declare -p \"${__this}\"" &>/dev/null; then
			break
		fi
		[[ -z "${!__this}" ]] && break
		__this="${!__this}"
	done
	local __vTable="${__this}____vPointer"
	if ! eval "declare -p \"${__vTable}\"" &>/dev/null; then
		return 1
	fi
	while [[ ! -z "${!__vTable}" ]] 2>/dev/null; do
		[[ "${!__vTable}" == "bpp__${__type}____vTable" ]] && eval "${__outputVar}=\"${__this}\"" && return 0
		__vTable="${!__vTable}[\"__parent__\"]"
	done
	return 1
}
)EOF";

[[maybe_unused]] static const char* bpp_typeof_function = R"EOF(function bpp____typeof() {
	local __this="$1" __outputVar="$2"
	[[ -z "${__this}" ]] && >&2 echo "Bash++: Error: Invalid type name request" && exit 1
	while : ; do
		if ! eval "declare -p \"${__this}\"" &>/dev/null; then
			break
		fi
		[[ -z "${!__this}" ]] && break
		__this="${!__this}"
	done
	local __vTable="${__this}____vPointer"
	if ! eval "declare -p \"${__vTable}\"" &>/dev/null; then
		return 1
	fi
	__vTable="${!__vTable}"
	local __typeName="${__vTable/bpp__/}"
	__typeName="${__typeName/____vTable/}"
	eval "${__outputVar}=\"${__typeName}\""
}
)EOF";

[[maybe_unused]] static const char* template_new_function = R"EOF(function bpp__%CLASS%____new() {
	local __this="$1"
	if [[ "${__this}" == "" ]]; then
		while : ; do
			__this="bpp__%CLASS%__$RANDOM$RANDOM$RANDOM$RANDOM"
			local __unusedVar="${__this}____vPointer"
			[[ -z "${!__unusedVar+x}" ]] && break
		done
	fi
	eval "${__this}____vPointer=bpp__%CLASS%____vTable"
%ASSIGNMENTS%
	echo "${__this}"
}
)EOF";

[[maybe_unused]] static const char* template_copy_function = R"EOF(function bpp__%CLASS%____copy() {
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
	if [[ "${!__copyFromVPointer}" != "${!__copyToVPointer}" ]]; then
		>&2 echo "Bash++: Error: %CLASS%: Attempted to copy between different types"
		return
	fi
%COPIES%
	eval "${__copyToVPointer}=${!__copyFromVPointer}"
}
)EOF";

[[maybe_unused]] static const char* template_method = R"EOF(function bpp__%CLASS%__%SIGNATURE%() {
	local __this="$1"
	shift 1
	%PARAMS%
	while : ; do
		if ! eval "declare -p \"${__this}\"" &>/dev/null; then
			break
		fi
		[[ -z "${!__this}" ]] && break
		__this="${!__this}"
	done
	local __vPointer="${__this}____vPointer"
	if [[ "${__this}" == "0" ]] || [[ -z "${!__vPointer}" ]]; then
		>&2 echo "Bash++: Error: Attempted to call @%CLASS%.%SIGNATURE% on null object"
		return
	fi
%METHODBODY%
}
)EOF";

#endif // SRC_BPP_INCLUDE_TEMPLATES_H_
