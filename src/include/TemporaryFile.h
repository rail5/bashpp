/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <unistd.h>

class TemporaryFile : public std::ofstream {
	private:
		std::filesystem::path temp_file_path;
	public:
		TemporaryFile() {
			std::string temp_path = (std::filesystem::temp_directory_path() / "bashpp_temp_XXXXXX").string();
			std::vector<char> temp_path_vector;
			temp_path_vector.reserve(temp_path.size() + 1);
			std::copy(temp_path.begin(), temp_path.end(), std::back_inserter(temp_path_vector));
			temp_path_vector.push_back('\0'); // Null-terminate the string for mkstemp

			int fd = mkstemp(temp_path_vector.data());
			if (fd == -1) {
				throw std::runtime_error("Could not create temporary file");
			}
			::close(fd);
			temp_file_path = temp_path_vector.data();
			this->open(temp_file_path, std::ios::out | std::ios::trunc);
			if (!this->is_open()) {
				throw std::runtime_error("Could not open temporary file for writing");
			}
		}

		~TemporaryFile() override {
			this->close();
			std::filesystem::remove(temp_file_path);
		}

		// Delete copy constructor and copy assignment operator to prevent copying
		TemporaryFile(const TemporaryFile&) = delete;
		TemporaryFile& operator=(const TemporaryFile&) = delete;

		// Delete move constructor and move assignment operator to prevent moving
		TemporaryFile(TemporaryFile&&) = delete;
		TemporaryFile& operator=(TemporaryFile&&) = delete;

		const std::filesystem::path& path() const {
			return temp_file_path;
		}
};
