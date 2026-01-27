/**
 * Copyright (C) 2023-2025 rail5
 */

#pragma once

#include <string>
#include <vector>

std::vector<std::string> explode(
	const std::string& input,
	char delimiter,
	bool can_escape = false,
	int maximum_number_of_elements = 0,
	bool preserve_empty = false
);
