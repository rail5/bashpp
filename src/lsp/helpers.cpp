/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include "helpers.h"

// TODO(@rail5): Refactor signal handling
// Probably end up using a dedicated signal handling thread

void signal_handler(int signum) {
	if (p_server) p_server->cleanup();
	_exit(signum);
}
