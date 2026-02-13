/**
 * Copyright (C) 2025-2026 Andrew S. Rightenburg
 *
 * XGetOpt is a C++20 compile-time wrapper around getopt_long for parsing command-line options.
 * It allows defining options at compile-time and automatically generates help strings without runtime overhead.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef XGETOPT_H_
#define XGETOPT_H_

// Throw a compiler error if the C++ version is less than C++20
#if __cplusplus < 202002L
#error "xgetopt.h requires C++20 or later"
#endif

#include <getopt.h>
#include <array>
#include <vector>
#include <string_view>
#include <string>
#include <optional>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <algorithm>
#include <concepts>
#include <stdexcept>
#include <utility>

namespace XGetOpt {
namespace Helpers {

constexpr bool is_ws(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

/**
 * @struct FixedString
 * @brief A fixed-size string class for compile-time string manipulation
 * 
 * @tparam N The maximum size of the string (including NUL terminator)
 */
template<size_t N>
struct FixedString {
		std::array<char, N> data{};
		size_t size = 0;
		constexpr FixedString() = default;
		constexpr FixedString(const char (&str)[N]) {
			while (size + 1 < N && str[size] != '\0') {
				data[size] = str[size];
				size++;
			}
			// Ensure NUL termination when possible
			if (size < N) data[size] = '\0';
		}
		constexpr ~FixedString() = default;
		
		constexpr const char* c_str() const {
			return data.data();
		}

		constexpr void append(const char* str) {
			size_t index = 0;
			while (str[index] != '\0' && size < N) {
				data[size++] = str[index++];
			}
		}
		constexpr void append(char c) {
			if (size < N) {
				data[size++] = c;
			}
		}
		constexpr void append(char c, size_t count) {
			for (size_t i = 0; i < count && size < N; i++) {
				data[size++] = c;
			}
		}
		constexpr void append(std::string_view sv) {
			for (size_t i = 0; i < sv.size() && size < N; i++) {
				data[size++] = sv[i];
			}
		}
		
		constexpr std::string_view view() const {
			return std::string_view(data.data(), size);
		}

		constexpr operator std::string_view() const {
			return view();
		}

		constexpr size_t length() const {
			return size;
		}

		constexpr std::optional<size_t> find(char c, size_t start = 0) const {
			for (size_t i = start; i < size; i++) {
				if (data[i] == c) {
					return i;
				}
			}
			return std::nullopt;
		}

		constexpr std::optional<size_t> find_first_of(const char* chars, size_t start = 0) const {
			for (size_t i = start; i < size; i++) {
				for (size_t j = 0; chars[j] != '\0'; j++) {
					if (data[i] == chars[j]) {
						return i;
					}
				}
			}
			return std::nullopt;
		}

		/**
		 * @brief Get the next word from the string starting at position pos
		 *
		 * A 'word' is considered to be a sequence of non-whitespace characters.
		 * 
		 * @param pos The starting position to search from
		 * @return constexpr std::string_view The next word found, or an empty string_view if none found
		 */
		constexpr std::string_view get_next_word(size_t pos) const {
			size_t next_whitespace_position = find_first_of(" \t\n", pos).value_or(size);
			return view().substr(pos, next_whitespace_position - pos);
		}
};

// Deduction guide for FixedString
template<size_t N>
FixedString(const char (&)[N]) -> FixedString<N>;

/**
 * @struct TextView
 * @brief A simple view over a text string
 * 
 */
struct TextView {
	const char* data = nullptr;
	size_t len = 0;

	constexpr size_t length() const { return len; }

	constexpr const char* c_str() const { return data; } // ptr must be NUL-terminated if used as C-string

	constexpr std::string_view view() const {
		return (data && len) ? std::string_view(data, len) : std::string_view{};
	}

	constexpr operator std::string_view() const { return view(); }

	constexpr std::string_view get_next_word(size_t pos) const {
		auto sv = view();
		if (pos >= sv.size()) return {};
		size_t end = pos;
		while (end < sv.size()) {
			char ch = sv[end];
			if (ch == ' ' || ch == '\t' || ch == '\n') break;
			++end;
		}
		return sv.substr(pos, end - pos);
	}
};

} // namespace Helpers

/**
 * @enum ArgumentRequirement
 * @brief Specifies whether an option requires an argument, has an optional argument, or has no argument
 * 
 */
enum ArgumentRequirement : uint8_t {
	NoArgument = 0,
	RequiredArgument = 1,
	OptionalArgument = 2
};

/**
 * @struct Option
 * @brief Compile-time representation of a command-line option
 * 
 * @tparam ShortOpt The short option character (e.g., 'h' for -h, or a unique integer for long-only options)
 * @tparam LongOpt The long option string (e.g., "help" for --help, or an empty string for short-only options)
 * @tparam Description The description of the option for help string generation
 * @tparam ArgReq The argument requirement (NoArgument, RequiredArgument, OptionalArgument)
 * @tparam ArgumentPlaceholder The placeholder text for the argument in help strings (default: "arg")
 */
template<
	int ShortOpt,
	Helpers::FixedString LongOpt,
	Helpers::FixedString Description,
	ArgumentRequirement ArgReq,
	Helpers::FixedString ArgumentPlaceholder = "arg">
struct Option {
	static constexpr int shortopt = ShortOpt;
	static constexpr auto longopt = LongOpt;
	static constexpr auto description = Description;
	static constexpr ArgumentRequirement argRequirement = ArgReq;
	static constexpr auto argumentPlaceholder = ArgumentPlaceholder;
};

namespace Helpers {

struct OptionView {
	int shortopt = 0;
	TextView longopt{};
	TextView description{};
	ArgumentRequirement argRequirement = ArgumentRequirement::NoArgument;
	TextView argumentPlaceholder{};
};

template<typename T>
concept option_like = requires(T t) {
	{ t.shortopt } -> std::convertible_to<int>;
	{ t.argRequirement } -> std::convertible_to<ArgumentRequirement>;
	{ t.longopt.length() } -> std::convertible_to<size_t>;
	{ t.description.length() } -> std::convertible_to<size_t>;
	{ t.argumentPlaceholder.length() } -> std::convertible_to<size_t>;
	{ t.description.get_next_word(size_t{}) } -> std::same_as<std::string_view>;
};

/**
 * @brief Calculate the length of the option label for help string formatting
 * 
 * @tparam T The option-like type
 * @param option The option to calculate the label length for
 * @return constexpr size_t The length of the option label
 */
template<option_like T>
constexpr size_t option_label_length(const T& option) {
	/**
	 * The option label format:
	 *  1. -x, --longopt <arg>
	 *  2. -x, --longopt[=arg]
	 *  3. -x, --longopt
	 *  4. -x <arg>
	 *  5. -x[arg]
	 *  6. -x
	 *  7.     --longopt <arg>
	 *  8.     --longopt[=arg]
	 *  9.     --longopt
	 *
	 * Where <arg> is used for required arguments and [=arg] is used for optional arguments.
	 * And where '-x' is the option's actual shortopt character,
	 * '--longopt' is the option's actual longopt string,
	 * and 'arg' is the option's argument placeholder string if provided, or 'arg' if not provided.
	 */
	size_t length = 2; // "-x" or equivalent whitespace if no shortopt

	if (option.longopt.length() > 0) {
		length += 2; // ", " or equivalent whitespace if no shortopt

		length += 2 + option.longopt.length(); // For "--longopt"
	}

	switch (option.argRequirement) {
		case XGetOpt::ArgumentRequirement::RequiredArgument:
			length += 2 // " <"
				+ option.argumentPlaceholder.length() + 1; // "arg>"
			break;
		case XGetOpt::ArgumentRequirement::OptionalArgument:
			length += 1; // "["
			if (option.longopt.length() > 0) {
				length += 1; // "="
			}
			length += option.argumentPlaceholder.length() + 1; // "arg]"
			break;
		case XGetOpt::ArgumentRequirement::NoArgument:
		default:
			break;
	}

	//length += 1; // Null terminator

	return length;
}

/**
 * @brief Calculate the maximum option label length among a set of options
 * 
 * @tparam N The number of options
 * @tparam T The option-like type
 * @param options The array of options to calculate the maximum label length for
 * @return constexpr size_t The maximum option label length
 */
template<size_t N, option_like T>
constexpr size_t max_option_label_length(const std::array<T, N>& options) {
	size_t max_length = 0;
	for (size_t i = 0; i < N; i++) {
		size_t length = option_label_length(options[i]);
		if (length > max_length) {
			max_length = length;
		}
	}
	return max_length;
}

/**
 * @brief Calculate the length of the help string for a set of options
 * 
 * @tparam N The number of options
 * @tparam T The option-like type
 * @param options The array of options to calculate the help string length for
 * @return constexpr size_t The length of the help string
 */
template<size_t N, option_like T>
constexpr size_t calculate_help_string_length(const std::array<T, N>& options) {
	size_t total_length = 0;
	const size_t max_label_length = max_option_label_length(options);

	for (size_t i = 0; i < N; i++) {
		size_t line_length = 2;
		total_length += 2; // indentation
		const size_t label_length = option_label_length(options[i]);
		const size_t padding_amount = max_label_length - label_length;

		total_length += label_length + padding_amount + 1;
		line_length += label_length + padding_amount + 1;

		const size_t line_limit = 80;
		size_t pos = line_length;

		auto desc = options[i].description.view();
		size_t idx = 0;

		while (idx < desc.size()) {
			// Skip whitespace
			while (idx < desc.size() && is_ws(desc[idx])) ++idx;
			if (idx >= desc.size()) break;

			// Find word end
			size_t end = idx;
			while (end < desc.size() && !is_ws(desc[end])) ++end;

			const size_t word_len = end - idx;

			// Wrap if the word doesn't fit on this line (and we're not at the start of the desc column)
			const size_t desc_col = max_label_length + 3;
			if (pos + word_len > line_limit && pos > desc_col) {
				total_length += 1;              // '\n'
				total_length += desc_col;       // indentation to description column
				pos = desc_col;
			}

			total_length += word_len;
			pos += word_len;

			// Look ahead: is there another word?
			size_t next = end;
			while (next < desc.size() && is_ws(desc[next])) ++next;
			if (next < desc.size()) {
				// Add a single separating space if it fits, else wrap
				if (pos + 1 > line_limit) {
					total_length += 1;
					total_length += desc_col;
					pos = desc_col;
				} else {
					total_length += 1;
					pos += 1;
				}
			}

			idx = end;
		}

		total_length += 1; // newline after each option
	}

	return total_length + 1; // trailing '\0'
}

} // namespace Helpers


/**
 * @class ParsedOption
 * @brief Represents a parsed command-line option and its associated argument (if any)
 * 
 */
class ParsedOption {
	private:
		int shortopt;
		std::optional<std::string_view> argument;
	public:
		ParsedOption(int s, std::optional<std::string_view> arg)
			: shortopt(s), argument(arg) {}
		
		int getShortOpt() const {
			return shortopt;
		}

		bool hasArgument() const {
			return argument.has_value();
		}

		std::string_view getArgument() const {
			return argument.value_or(std::string_view{});
		}
};

/**
 * @class OptionSequence
 * @brief Represents a sequence of parsed options and non-option arguments
 * 
 */
class OptionSequence {
	private:
		std::vector<ParsedOption> options;
		std::vector<std::string_view> nonOptionArguments;

		/**
		 * @brief Add another OptionSequence to this one
		 *
		 * The options and non-option arguments from the other sequence are appended to this one.
		 * This is accessible via operator+= and operator+.
		 * 
		 * @param other The other OptionSequence to add
		 */
		void add(const OptionSequence& other) {
			options.insert(options.end(), other.options.begin(), other.options.end());
			nonOptionArguments.insert(nonOptionArguments.end(),
				other.nonOptionArguments.begin(), other.nonOptionArguments.end());
		}
	public:
		void addOption(const ParsedOption& opt) {
			options.push_back(opt);
		}
		void addNonOptionArgument(std::string_view arg) {
			nonOptionArguments.push_back(arg);
		}

		OptionSequence& operator+=(const OptionSequence& rhs) {
			add(rhs);
			return *this;
		}
		friend OptionSequence operator+(OptionSequence lhs, const OptionSequence& rhs) {
			lhs.add(rhs);
			return lhs;
		}

		auto begin() const {
			return options.begin();
		}
		auto end() const {
			return options.end();
		}
		size_t size() const {
			return options.size();
		}
		bool empty() const {
			return options.empty();
		}
		const ParsedOption& operator[](size_t index) const {
			return options[index];
		}
		const ParsedOption& at(size_t index) const {
			return options.at(index);
		}

		/**
		 * @brief Check if an option with the given shortopt was provided
		 * 
		 * @param shortopt The shortopt character or unique integer identifier to check for
		 * @return true if the option was provided
		 * @return false if the option was not provided
		 */
		bool hasOption(int shortopt) const {
			return std::any_of(options.begin(), options.end(),
				[shortopt](const ParsedOption& opt) {
					return opt.getShortOpt() == shortopt;
				});
		}

		/**
		 * @brief Get a std::vector of all non-option arguments provided, in the order they were given
		 * 
		 * @return const std::vector<std::string_view>& The vector of non-option arguments
		 */
		const std::vector<std::string_view>& getNonOptionArguments() const {
			return nonOptionArguments;
		}
};

/**
 * @enum StopCondition
 * @brief Specifies when to stop parsing options
 *
 * An explanation of each:
 *  - AllOptions:
 *      Parse all options until the end of the argument list
 *
 *  - BeforeFirstNonOptionArgument:
 *      Stop parsing when the first non-option argument is encountered
 *      The non-option argument and any subsequent arguments are left unparsed
 *
 *  - AfterFirstNonOptionArgument:
 *      Stop parsing after the first non-option argument is encountered
 *      The first non-option argument is included in the parsed results,
 *      but any subsequent arguments are left unparsed
 *
 *  - BeforeFirstError:
 *     Stop parsing when the first error is encountered (unknown option or missing required argument)
 *     The offending option is not included in the parsed results,
 *     and it as well as any subsequent arguments are left unparsed
 * 
 */
enum StopCondition {
	AllOptions,
	BeforeFirstNonOptionArgument,
	AfterFirstNonOptionArgument,
	BeforeFirstError
};

struct OptionRemainder {
	int argc;
	char** argv;
};

/**
 * @class OptionParser
 * @brief Parses command-line arguments based on a set of defined options
 * 
 * @tparam Options The variadic list of option definitions
 */
template<Helpers::option_like... Options>
class OptionParser {
	private:
		static constexpr size_t N = sizeof...(Options);
		using OptionArray = std::array<Helpers::OptionView, N>;

		static constexpr OptionArray options = OptionArray{{
			Helpers::OptionView{
				Options::shortopt,
				Helpers::TextView{ Options::longopt.c_str(), Options::longopt.length() },
				Helpers::TextView{ Options::description.c_str(), Options::description.length() },
				Options::argRequirement,
				Helpers::TextView{ Options::argumentPlaceholder.c_str(), Options::argumentPlaceholder.length() }
			}...
		}};

		// Compile-time guarantee: no two options have the same shortopt value
		static_assert([]{
			for (size_t i = 0; i < N; i++) {
				for (size_t j = i + 1; j < N; j++) {
					if (options[i].shortopt == options[j].shortopt) {
						return false;
					}
				}
			}
			return true;
		}(), "OptionParser error: Duplicate shortopt values detected in option definitions.");

		// Likewise: no two options have the same (non-empty) longopt value
		static_assert([]{
			for (size_t i = 0; i < N; i++) {
				if (options[i].longopt.length() == 0) continue;
				for (size_t j = i + 1; j < N; j++) {
					if (options[j].longopt.length() == 0) continue;
					if (options[i].longopt.view() == options[j].longopt.view()) {
						return false;
					}
				}
			}
			return true;
		}(), "OptionParser error: Duplicate longopt values detected in option definitions.");

		static constexpr size_t help_string_length = Helpers::calculate_help_string_length<N>(options);
		
		static constexpr std::array<char, 3*N + 2> build_short_options_(const OptionArray& opts) {
			size_t short_opt_index = 0;
			std::array<char, 3*N + 2> short_opts{};

			// Flag: REQUIRE_ORDER
			// This means that option processing stops at the first non-option argument as mandated by POSIX
			// Represented by a leading '+' in the short options string
			// This flag is supported in the vast majority of getopt implementations
			// Including GNU libc, every BSD variant, and musl, among others.
			short_opts[short_opt_index++] = '+';

			for (size_t i = 0; i < N; i++) {
				if (opts[i].shortopt >= 33 && opts[i].shortopt <= 126) {
					short_opts[short_opt_index++] = static_cast<char>(opts[i].shortopt);
					switch (opts[i].argRequirement) {
						case XGetOpt::ArgumentRequirement::RequiredArgument:
							short_opts[short_opt_index++] = ':';
							break;
						case XGetOpt::ArgumentRequirement::OptionalArgument:
							short_opts[short_opt_index++] = ':';
							short_opts[short_opt_index++] = ':';
							break;
						case XGetOpt::ArgumentRequirement::NoArgument:
						default:
							// No extra characters needed
							break;
					}
				}
			}

			// Null-terminate the array
			short_opts[short_opt_index] = '\0';
			return short_opts;
		}

		static constexpr std::array<struct option, N + 1> build_long_options_(const OptionArray& opts) {
			std::array<struct option, N + 1> long_opts{};

			size_t idx = 0;

			for (size_t i = 0; i < N; i++) {
				if (opts[i].longopt.length() == 0) continue;
				long_opts[idx].name = opts[i].longopt.c_str();
				long_opts[idx].has_arg = static_cast<int>(opts[i].argRequirement);
				long_opts[idx].flag = nullptr;
				long_opts[idx].val = opts[i].shortopt;
				idx++;
			}

			// Null-terminate the array (GNU getopt expects a sentinel)
			long_opts[idx] = {nullptr, 0, nullptr, 0};
			return long_opts;
		}

		static constexpr std::array<char, 3*N + 2> short_options = build_short_options_(options);
		static constexpr std::array<struct option, N + 1> long_options = build_long_options_(options);

		static constexpr Helpers::FixedString<help_string_length> generateHelpString(const OptionArray& options) {
			Helpers::FixedString<help_string_length> help_string;
			const size_t max_label_length = Helpers::max_option_label_length<N>(options);

			for (size_t i = 0; i < N; i++) {
				help_string.append("  ");

				// First: write the '-x, --longopt <arg>' part
				if (options[i].shortopt >= 33 && options[i].shortopt <= 126) {
					help_string.append('-');
					help_string.append(static_cast<char>(options[i].shortopt));
					if (options[i].longopt.length() > 0) {
						help_string.append(", ");
					}
				} else {
					// Longopt only, add spaces for alignment
					help_string.append(' ', 4); // The space that would've been used by "-x, "
				}

				if (options[i].longopt.length() > 0) {
					help_string.append("--");
					help_string.append(options[i].longopt);
				}

				switch (options[i].argRequirement) {
					case XGetOpt::ArgumentRequirement::RequiredArgument:
						help_string.append(" <");
						help_string.append(options[i].argumentPlaceholder);
						help_string.append(">");
						break;
					case XGetOpt::ArgumentRequirement::OptionalArgument:
						// Shortopt-only? "[arg]"
						// Longopt (with or without shortopt)? "[=arg]"
						help_string.append("[");
						if (options[i].longopt.length() > 0) {
							help_string.append("=");
						}
						help_string.append(options[i].argumentPlaceholder);
						help_string.append("]");
						break;
					case XGetOpt::ArgumentRequirement::NoArgument:
					default:
						break;
				}

				// Second: write the option description

				const size_t label_length = option_label_length(options[i]);
				const size_t padding_amount = max_label_length - label_length;
				help_string.append(' ', padding_amount + 1);

				const size_t line_limit = 80;
				const size_t desc_col = max_label_length + 3;
				size_t pos = 2 + max_label_length + 1;

				auto desc = options[i].description.view();
				size_t idx = 0;

				while (idx < desc.size()) {
					while (idx < desc.size() && Helpers::is_ws(desc[idx])) ++idx;
					if (idx >= desc.size()) break;

					size_t end = idx;
					while (end < desc.size() && !Helpers::is_ws(desc[end])) ++end;

					std::string_view word = desc.substr(idx, end - idx);

					if (pos + word.size() > line_limit && pos > desc_col) {
						help_string.append('\n');
						help_string.append(' ', desc_col);
						pos = desc_col;
					}

					help_string.append(word);
					pos += word.size();

					size_t next = end;
					while (next < desc.size() && Helpers::is_ws(desc[next])) ++next;
					if (next < desc.size()) {
						if (pos + 1 > line_limit) {
							help_string.append('\n');
							help_string.append(' ', desc_col);
							pos = desc_col;
						} else {
							help_string.append(' ');
							pos += 1;
						}
					}

					idx = end;
				}

				help_string.append('\n');
			}

			return help_string;
		}

		static constexpr Helpers::FixedString<help_string_length> help_string
			= generateHelpString(options);

		template <StopCondition parseUntil>
		std::pair<OptionSequence, OptionRemainder> parse_impl(int argc, char* argv[]) const {
			OptionSequence parsed_options;
			OptionRemainder unparsed_options{argc, argv};

			// Reset in case parse() is called more than once in a process.
			opterr = 0; // Don't let getopt print messages.
			optind = 1;
			
			// Platform-specific:
			// GNU or Haiku: Set optind = 0 to reset getopt state fully
			// Any of the BSDs or musl: Set optreset = 1 to reset getopt state fully
			//
			// Setting optind = 0 is UB outside of GNU/Haiku, so only do it there.
			//
			// GNU and Haiku have no concept of "optreset"
			//
			// On any unidentified platforms, we'll just have to cross our fingers.
			//
			// TODO(@rail5): Solaris?
		#if defined(__GLIBC__) || defined(__HAIKU__)
			optind = 0;
		#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__MUSL__)
			optreset = 1;
		#endif

			int remainder_start = -1;

			auto find_by_short = [&](int s) -> const auto* {
				for (size_t i = 0; i < N; i++) {
					if (options[i].shortopt == s) return &options[i];
				}
				return static_cast<const Helpers::OptionView*>(nullptr);
			};

			auto find_by_long = [&](std::string_view name) -> const auto* {
				for (size_t i = 0; i < N; i++) {
					if (options[i].longopt.length() > 0
						&& name == std::string_view(options[i].longopt.data, options[i].longopt.length())) {
						return &options[i];
					}
				}
				return static_cast<const Helpers::OptionView*>(nullptr);
			};

			auto token_to_long_name = [&](const char* tok) -> std::string_view {
				if (!tok) return {};
				std::string_view sv(tok);
				if (sv.rfind("--", 0) != 0) return {};
				sv.remove_prefix(2);
				if (auto eq = sv.find('='); eq != std::string_view::npos) {
					sv = sv.substr(0, eq);
				}
				return sv;
			};

			int longindex = 0;

			for (;;) {
				const int token_index = (optind == 0) ? 1 : optind; // The argv examined by this call
				const int opt = getopt_long(argc, argv, short_options.data(), long_options.data(), &longindex);
				if (opt == -1) {
					if (optind >= argc) break; // All tokens processed

					// Special-case:
					// `--` ends option parsing.
					// getopt_long consumes it, sets optind to the first token after `--`,
					// and then returns -1.
					// In our case, we still want to collect the remaining tokens as non-option arguments.
					if (optind > 0 && std::string_view(argv[optind - 1]) == "--") {
						if constexpr (parseUntil == BeforeFirstNonOptionArgument) {
							remainder_start = token_index;
							break;
						} else if constexpr (parseUntil == AfterFirstNonOptionArgument) {
							// Include the first non-option argument (which is argv[optind])
							parsed_options.addNonOptionArgument(std::string_view(argv[optind]));
							optind++; // Consume
							remainder_start = optind;
							break;
						}
						// Otherwise (AllOptions or BeforeFirstError), collect all remaining tokens
						for (int i = optind; i < argc; i++) {
							parsed_options.addNonOptionArgument(std::string_view(argv[i]));
						}
						optind = argc; // All tokens consumed
						break;
					}

					// If we're here, we have a non-option argument to process
					if constexpr (parseUntil == BeforeFirstNonOptionArgument) {
						remainder_start = token_index;
						break;
					} else if constexpr (parseUntil == AfterFirstNonOptionArgument) {
						parsed_options.addNonOptionArgument(std::string_view(argv[optind]));
						optind++; // Consume
						remainder_start = optind;
						break;
					}

					// Otherwise, continue processing
					// AllOptions or BeforeFirstError
					parsed_options.addNonOptionArgument(std::string_view(argv[optind]));
					optind++; // Consume
					continue;
				}

				// Error handling
				if (opt == '?') {
					if constexpr (parseUntil == BeforeFirstError) {
						remainder_start = token_index; // include offending token in remainder
						break;
					}

					// Use token_index for the culprit token deterministically:
					const char* culprit_tok =
						(token_index >= 0 && token_index < argc) ? argv[token_index] : nullptr;

					// For shortopts missing arg: optopt is set to the offending option character.
					if (optopt != 0) {
						const auto* o = find_by_short(optopt);
						if (o && o->argRequirement == XGetOpt::ArgumentRequirement::RequiredArgument) {
							std::string option_name;
							if (optopt >= 33 && optopt <= 126) {
								option_name += '-';
								option_name += static_cast<char>(optopt);
							} else {
								option_name = std::string(o->longopt.length() > 0
									? o->longopt.data
									: "???");
							}
							throw std::runtime_error(std::string("Missing required argument for option: ")
								+ option_name);
						}
					}

					// For long options missing arg: argv[optind-1] is typically the option token.
					const std::string_view long_name = token_to_long_name(culprit_tok);
					if (!long_name.empty()) {
						const auto* o = find_by_long(long_name);
						if (o && o->argRequirement == XGetOpt::ArgumentRequirement::RequiredArgument) {
							throw std::runtime_error(std::string("Missing required argument for option: --")
								+ std::string(long_name));
						}
					}

					// Otherwise: unknown option
					if (culprit_tok) {
						throw std::runtime_error(std::string("Unknown option: ") + culprit_tok);
					}

					// Last resort:
					throw std::runtime_error("Unknown option");
				}

				std::optional<std::string_view> argument;
				if (optarg != nullptr) argument = std::string_view(optarg);
				parsed_options.addOption(ParsedOption(opt, argument));
			}

			const int start = (remainder_start >= 0) ? remainder_start : optind;
			unparsed_options.argc = argc - start;
			unparsed_options.argv = &argv[start];
			return {parsed_options, unparsed_options};
		}
	public:
		/**
		 * @brief Get the compile-time generated help string
		 * 
		 * @return constexpr std::string_view The help string for this option parser detailing all options
		 */
		constexpr std::string_view getHelpString() const {
			return help_string.view();
		}

		constexpr const OptionArray& getOptions() const {
			return options;
		}

		/**
		 * @brief Parse command-line arguments according to the defined options
		 * 
		 * @param argc The argument count
		 * @param argv The argument vector
		 * @return OptionSequence The sequence of parsed options and non-option arguments
		 */
		OptionSequence parse(int argc, char* argv[]) const {
			return parse_impl<AllOptions>(argc, argv).first;
		}

		/**
		 * @brief Parse command-line arguments until a specified condition is met
		 *
		 * This overload allows specifying a condition to stop parsing early,
		 * such as stopping at the first non-option argument or the first error.
		 * 
		 * @tparam end The condition to stop parsing at
		 * @param argc The argument count
		 * @param argv The argument vector
		 * @return std::pair<OptionSequence, OptionRemainder> A pair containing the parsed options and the remaining unparsed arguments
		 */
		template<StopCondition end>
		std::pair<OptionSequence, OptionRemainder> parse_until(int argc, char* argv[]) const {
			return parse_impl<end>(argc, argv);
		}
};

} // namespace XGetOpt

#endif // XGETOPT_H_
