#include <stdexcept>

struct syntax_error : public std::runtime_error {
	syntax_error(const std::string& msg, std::string source_file, int line_number) : std::runtime_error(source_file + ":" + std::to_string(line_number) + ": " + msg) {}
};