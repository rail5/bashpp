/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <string>
#include <vector>
#include <cerrno>
#include <system_error>

#include <sys/wait.h>
#include <unistd.h>

#include <error/InternalError.h>

/**
 * @brief Runs a Bash script with the given filename and arguments, and returns the exit code of the script.
 * 
 * @param filename 
 * @param arguments 
 * @return int 
 */
inline int run_bash(const std::string& filename, const std::vector<char*>& arguments) {
	std::vector<std::string> exec_args;
		exec_args.reserve(2 + arguments.size());
		exec_args.emplace_back("bash");
		exec_args.emplace_back(filename);
		for (auto* argument : arguments) {
			exec_args.emplace_back(argument ? argument : "");
		}

		std::vector<char*> argv;
		argv.reserve(exec_args.size() + 1);
		for (auto& s : exec_args) {
			argv.push_back(s.data());
		}
		argv.push_back(nullptr);

		pid_t child = fork();
		if (child < 0) {
			std::error_code ec(errno, std::generic_category());
			throw bpp::ErrorHandling::InternalError(std::string("fork() failed: ") + ec.message());
		}

		if (child == 0) {
			// We are now in the child process. Execute the compiled script.
			execvp(argv[0], argv.data());
			// If exec fails, we cannot throw (we're in the child) and we must not run
			// parent atexit handlers. Exit 127 is a widely-used convention for exec failure.
			_exit(127);
		}

		int status = 0;
		pid_t waited;
		do {
			waited = waitpid(child, &status, 0);
		} while (waited < 0 && errno == EINTR);

		if (waited < 0) {
			std::error_code ec(errno, std::generic_category());
			throw bpp::ErrorHandling::InternalError(std::string("waitpid() failed: ") + ec.message());
		}

		if (WIFEXITED(status)) {
			return WEXITSTATUS(status);
		} else if (WIFSIGNALED(status)) {
			// If the child died due to a signal, there is no exit-status byte to report.
			// There is a garbage "folklore" kind of rule that "most tools return 128 + signal number" in this case
			// No spec in the world describes this, and the only way to know that we're "supposed" to do it
			// is to be a member of their "boys' club."
			// Can you tell I hate things like this?
			// Undocumented folkloric conventions are what's wrong with FOSS & the primary mechanism for gatekeeping.
			// Anyway, principle of least surprise, we'll do the same thing.
			return 128 + WTERMSIG(status);
		} else {
			return EXIT_FAILURE;
		}
}
