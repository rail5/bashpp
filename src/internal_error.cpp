/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "internal_error.h"

internal_error::internal_error(const std::string& msg) : std::runtime_error(msg + "\nYou've found a bug! Please report it.") {}
