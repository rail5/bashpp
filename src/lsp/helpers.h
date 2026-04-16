/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <csignal>
#include <unistd.h>

#include "BashppServer.h"

extern bpp::BashppServer* p_server;

[[ noreturn ]] void signal_handler(int signum);
