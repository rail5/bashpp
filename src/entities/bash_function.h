/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "bpp.h"
#include "bpp_code_entity.h"

namespace bpp {

/**
 * @class bash_function
 * @brief A normal Bash function
 *
 * This entity gets pushed onto the entity stack when a normal Bash function is encountered in Bash++ code.
 *
 * The 'bash_' prefix signifies that this is used to parse ordinary Bash code, not anything specific to Bash++
 * 
 */
class bash_function : public bpp_code_entity {};

} // namespace bpp
