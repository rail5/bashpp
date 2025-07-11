/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <csignal>
#include <unistd.h>

#include "BashppServer.h"

extern BashppServer server;
extern int client_fd;
extern int server_fd;
extern int socket_port;

[[ noreturn ]] void signal_handler(int signum);
int setup_tcp_server(int port);
int setup_unix_socket_server(const std::string& path);
