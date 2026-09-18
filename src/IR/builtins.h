/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string_view>

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
 *
 * - The '-v' option was added to the printf builtin in Bash 3.1.
 *   printf -v is used in almost everything
 */

namespace bpp::IR::Builtins {

[[maybe_unused]] constexpr static std::string_view bpp_supershell_function = R"EOF(bpp____initsupershell() {
	local bpp____supershellDirectory="/dev/shm/"
	if [[ ! -d "${bpp____supershellDirectory}" ]]; then
		bpp____supershellDirectory="${TMPDIR:-/tmp/}"
	fi
	local bpp____supershelltempfile="$(mktemp "${bpp____supershellDirectory}/XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")"
	eval "exec {bpp____supershellFD__$BASHPID}<>\"$bpp____supershelltempfile\""
	rm "$bpp____supershelltempfile"
}
bpp____supershell() {
	local __outputVar="$1" __command="$2" __supershellFD="bpp____supershellFD__$BASHPID" __temporaryStorage=""
	if [[ -z "${!__supershellFD}" ]]; then
		bpp____initsupershell
	else
		__temporaryStorage=$(< "/dev/fd/${!__supershellFD}")
	fi
	$__command 1>"/dev/fd/${!__supershellFD}"
	printf -v "${__outputVar}" '%s' "$(< "/dev/fd/${!__supershellFD}")"
	echo "${__temporaryStorage}">"/dev/fd/${!__supershellFD}"
}
)EOF";

[[maybe_unused]] constexpr static std::string_view bpp_repeat_function = R"EOF(bpp____repeat() {
	return $1
}
)EOF";

[[maybe_unused]] constexpr static std::string_view bpp_vtable_lookup_function = R"EOF(bpp____vTable_lookup() {
	local __this="$1" __method="$2" __outputVar="$3"
	if [[ -z "${__this}" ]] || [[ -z "${__method}" ]] || [[ -z "${__outputVar}" ]]; then
		>&2 echo "Bash++: Error: Invalid vTable lookup"
		exit 1
	fi
	printf -v "${__outputVar}" '%s' 0
	while [[ -v "${__this}" ]] && [[ ! -z "${__this}" ]]; do
		__this="${!__this}"
	done
	local __vTable="${__this}____vPointer"
	if [[ ! -v "${__vTable}" ]]; then
		>&2 echo "Bash++: Error: Object '${__this}' has no vTable pointer" && return 1
	fi
	local __result="${!__vTable}[\"${__method}\"]"
	if [[ -z "${!__result}" ]]; then
		>&2 echo "Bash++: Error: Method '${__method}' not found in vTable for object '${__this}'"
		return 1
	fi
	__result=${!__result}
	printf -v "${__outputVar}" '%s' "${__result}"
}
)EOF";

[[maybe_unused]] constexpr static std::string_view bpp_dynamic_cast_function = R"EOF(bpp____dynamic_cast() {
	local __type="$1" __outputVar="$2" __address="$3"
	if [[ -z "${__outputVar}" ]]; then
		>&2 echo "Bash++: Error: Invalid dynamic_cast"
		exit 1
	fi
	printf -v "${__outputVar}" '%s' 0
	while [[ -v "${__address}" ]] && [[ ! -z "${!__address}" ]]; do
		__address="${!__address}"
	done
	local __vTable="${__address}____vPointer"
	if [[ ! -v "${__vTable}" ]]; then
		return 1
	fi
	while [[ ! -z "${!__vTable}" ]] 2>/dev/null; do
		if [[ "${!__vTable}" == "bpp__${__type}____vTable" ]]; then
			printf -v "${__outputVar}" '%s' "${__address}"
			return 0
		fi
		__vTable="${!__vTable}[\"__parent__\"]"
	done
	return 1
}
)EOF";

[[maybe_unused]] constexpr static std::string_view bpp_typeof_function = R"EOF(bpp____typeof() {
	local __this="$1" __outputVar="$2"
	if [[ -z "${__this}" ]]; then
		>&2 echo "Bash++: Error: Invalid type name request"
		exit 1
	fi
	while [[ -v "${__this}" ]] && [[ ! -z "${!__this}" ]]; do
		__this="${!__this}"
	done
	local __vTable="${__this}____vPointer"
	if [[ ! -v "${__vTable}" ]]; then
		return 1
	fi
	__vTable="${!__vTable}"
	local __typeName="${__vTable/bpp__/}"
	__typeName="${__typeName/____vTable/}"
	printf -v "${__outputVar}" '%s' "${__typeName}"
}
)EOF";

} // namespace bpp::IR::Builtins
