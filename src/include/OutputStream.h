/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <filesystem>
#include <string>
#include <vector>
#include <functional>
#include <unistd.h>

namespace bpp::CodeGen {

using StreamPtr = std::unique_ptr<std::ostream, std::function<void(std::ostream*)>>;

template <typename T>
concept Streamable = requires(T a, std::ostream& os) {
	{ os << a } -> std::same_as<std::ostream&>;
};

/**
 * @brief A stream to which we write generated code.
 *
 * By default, this stream writes to stdout.
 *
 * Derived classes (OutputFile and TemporaryFile) write to a file instead.
 */
class OutputStream {
	public:
		friend OutputStream& operator<<(OutputStream& os, Streamable auto&& obj) {
			if (!os.initialized) os.initialize();
			(*os.stream) << std::forward<decltype(obj)>(obj);
			return os;
		}

		void flush() {
			if (!initialized) return;
			stream->flush();
		}

		OutputStream() {
			// If the user didn't provide an output file, default to stdout
			// Note that we use a no-op deleter here, so that the stream is not closed when the OutputStream is destroyed.
			stream = StreamPtr(&std::cout, [](std::ostream*){});
		}

		virtual ~OutputStream() = default;

		// Delete copy/move constructors and assignment operators to prevent copying/moving
		OutputStream(const OutputStream&) = delete;
		OutputStream& operator=(const OutputStream&) = delete;
		OutputStream(OutputStream&&) = delete;
		OutputStream& operator=(OutputStream&&) = delete;

	protected:
		StreamPtr stream;
		bool initialized = false;

		virtual void initialize() {
			// No-op in base class, derived classes override this to perform initialization
			// Initialization happens on first write, not on construction
			initialized = true;
		}
};

/**
 * @brief A stream for writing generated code to a file.
 *
 * The file is created/opened on first write, and closed on destruction.
 *
 * When the stream is destroyed, the file is marked as executable (if it was successfully created).
 */
class OutputFile : public OutputStream {
	public:
		const std::filesystem::path& path() const {
			return file_path;
		}

		~OutputFile() override {
			auto* fstream = static_cast<std::ofstream*>(stream.get());
			if (fstream && fstream->is_open()) {
				fstream->close();
			}

			// Mark the file executable
			if (initialized) try {
				std::filesystem::permissions(
					file_path,
					std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec | std::filesystem::perms::others_exec,
					std::filesystem::perm_options::add
				);
			} catch (...) { /* ignore */ }
		}

		OutputFile() = delete;
		explicit OutputFile(const std::filesystem::path& path) : file_path(path) {
			validate_path();
		}

		OutputFile(const OutputFile&) = delete;
		OutputFile& operator=(const OutputFile&) = delete;
		OutputFile(OutputFile&&) = delete;
		OutputFile& operator=(OutputFile&&) = delete;

	protected:
		std::filesystem::path file_path;

		void initialize() override {
			auto temp = std::make_unique<std::ofstream>(file_path, std::ios::out | std::ios::trunc);
			if (!temp->good()) {
				throw std::runtime_error("Could not open output file '" + file_path.string() + "' for writing");
			}
			stream = StreamPtr(temp.release(), std::default_delete<std::ostream>());
			initialized = true;
		}

		bool validate_path() const {
			// Verify that the file can be created/written to
			if (std::filesystem::exists(file_path)) {
				if (access(file_path.c_str(), W_OK) != 0) {
					throw std::runtime_error("No write permission for output file '" + file_path.string() + "'");
				}
			} else {
				std::filesystem::path parent_path = file_path.parent_path();
				if (!std::filesystem::exists(parent_path) || !std::filesystem::is_directory(parent_path)) {
					throw std::runtime_error("Parent directory of output file '" + file_path.string() + "' does not exist or is not a directory");
				}
				if (access(parent_path.c_str(), W_OK) != 0) {
					throw std::runtime_error("No write permission for parent directory of output file '" + file_path.string() + "'");
				}
			}

			return true;
		}
};

/**
 * @brief A stream for writing generated code to a temporary file.
 *
 * The temporary file is created on construction, truncated on first write, and deleted on destruction.
 * This is used when the user requested to run the compiled program on exit.
 */
class TemporaryFile : public OutputFile {
	public:
		TemporaryFile() : OutputFile(generate_path()) {}

		~TemporaryFile() override {
			auto* fstream = static_cast<std::ofstream*>(stream.get());
			if (fstream && fstream->is_open()) {
				fstream->close();
			}
			std::filesystem::remove(file_path);
		}

		TemporaryFile(const TemporaryFile&) = delete;
		TemporaryFile& operator=(const TemporaryFile&) = delete;
		TemporaryFile(TemporaryFile&&) = delete;
		TemporaryFile& operator=(TemporaryFile&&) = delete;

	private:
		static std::filesystem::path generate_path() {
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
			return {temp_path_vector.data()};
		}
};

} // namespace bpp::CodeGen
