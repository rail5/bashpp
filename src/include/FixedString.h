#pragma once

#include <array>
#include <string_view>
#include <ostream>

template<size_t N>
struct FixedString {
	std::array<char, N> data{};
	size_t size = 0;

	consteval void append(std::string_view str) {
		for (char c : str) {
			data[size++] = c;
		}
	}

	consteval void append(char c, size_t count = 1) {
		for (size_t i = 0; i < count; i++) {
			data[size++] = c;
		}
	}

	consteval std::string_view view() const {
		return std::string_view(data.data(), size);
	}

	friend std::ostream& operator<<(std::ostream& os, const FixedString& fs) {
		return os.write(fs.data.data(), fs.size);
	}
};

// Helper function to figure a string's length at compile-time
consteval size_t string_length(const char* str) {
	size_t length = 0;
	while (str && str[length] != '\0') {
		length++;
	}
	return length;
}
