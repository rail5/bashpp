#include <string>
#include <regex>
#include <set>
#include <vector>
#include <map>

#include "explode.cpp"
#include "replace_all.cpp"
#include "string_starts_with.cpp"

#include "bpp_templates.cpp"

struct bpp_param {
	std::string name = "";
	std::string type = "primitive";
};

struct bpp_method {
	std::string name = "";
	std::vector<bpp_param> parameters;
	std::string body = "";

	std::string generate_signature() const {
		std::string result = name;
		if (parameters.size() > 0) {
			result += "__";
		}
		for (const bpp_param& param : parameters) {
			result += "__" + param.type;
		}
		return result;
	}

	std::string handle_method_body() const {
		std::string result = "";

		bool in_string = false;
		// Replace unescaped semicolons (not within strings) with newlines

		std::string new_body = "";
		for (size_t i = 0; i < body.length(); i++) {
			if (body[i] == ';' && !in_string && (i == 0 || body[i - 1] != '\\')) {
				new_body += '\n';
			} else {
				new_body += body[i];
			}
			if (body[i] == '"' && (i == 0 || body[i - 1] != '\\')) {
				in_string = !in_string;
			}
		}

		std::vector<std::string> lines = explode(body, '\n');

		std::regex reference_pattern(R"EOF((?!.*__.*)(@\{*)(([a-zA-Z_]([a-zA-Z0-9_.]*)))(\}*))EOF");
		std::smatch match;

		in_string = false;
		bool previous_newline_escaped = false;
		for (std::string& line : lines) {
			std::set<std::string> resolved = {};
			std::string append = "";

			bool contains_unterminated_string = false;
			for (size_t i = 0; i < line.length(); i++) {
				if (line[i] == '"') {
					if (i == 0 || line[i - 1] != '\\') {
						in_string = !in_string;
					}
				}
				if (line[i] == '\n' && in_string) {
					contains_unterminated_string = true;
					break;
				}
			}

			while (std::regex_search(line, match, reference_pattern)) {
				auto match_start = match.position();
				auto match_end = match_start + match.length();
				std::string reference = match[2].str();

				// Handling '@this'
				if (reference == "this") {
					// Substitute '@this' with '${__objectAddress}'
					// No need to prepend anything to this line
					//line.replace(match_start, match_end, "${__objectAddress}");
					std::string new_line = line.substr(0, match_start);
					new_line += "${__objectAddress}";
					new_line += line.substr(match_end);
					line = new_line;
				} else if (string_starts_with(reference, "this.")) {
					// Is the reference an lvalue or an rvalue?
					bool is_lvalue = false;
					std::string assignment = "";
					size_t assignment_start = match_end + 1;
					size_t assignment_end = line.length();
					/**
					 * How can we tell if it's an lvalue?
					 * 1. It must be followed by an assignment operator
					 * 2. It must be the first part of the statement
					 * 		Note that statements can coexist on the same line, separated by semicolons
					 * 		Example: 'somecommand; @this.dataMember=5; anothercommand'
					 * 		We should also take care not be tricked by ESCAPED semicolons or escaped newlines
					 * 3. It must not be contained within a string
					 * 
					 * If all these conditions are met, then we can safely assume that the reference is an lvalue
					 */

					// Check if the reference is an lvalue
					// Are all the characters before the reference whitespace?
					bool all_whitespace = true;
					for (size_t i = 0; i < match_start; i++) {
						if (line[i] != ' ' && line[i] != '\t') {
							all_whitespace = false;
							break;
						}
					}
					if (all_whitespace && !previous_newline_escaped && !in_string) {
						// It's at the beginning of the line. Check if it's followed by an assignment operator
						if (line[match_end] == '=') {
							// It's an lvalue
							is_lvalue = true;

							// What are we assigning to it?
							// We need to find the end of the assignment
							// It could be a simple assignment, or a more complex one
							// E.g:
							//		'@this.member=5'
							//		'@this.member="string with spaces"'
							//		'@this.member=$(subshell with spaces)'
							//		'@this.member=@(supershell with spaces)'
							// So what's the procedure?
							/**
							 * How can we find the end of the assignment?
							 * 1. Look for quote marks. If we find one, we need to find the next one that isn't escaped
							 * 2. Otherwise, look if we can find a left parenthesis before whitespace. If we do, we need to find the matching right parenthesis
							 * 3. Otherwise, look for whitespace, or the end of the line
							 */
							bool in_quotes = false;
							int paren_depth = 0;

							for (size_t i = assignment_start; i < line.length(); i++) {
								switch (line[i]) {
									case '"':
										if (line[i - 1] != '\\') {
											in_quotes = !in_quotes;
										}
										break;
									case '(':
										if (!in_quotes) {
											paren_depth++;
										}
										break;
									case ')':
										if (!in_quotes && paren_depth > 0) {
											paren_depth--;
										}
										break;
									case ' ':
									case '\t':
										if (!in_quotes && paren_depth == 0) {
											assignment_end = i;
											break;
										}
										break;
								}
							}
							assignment = line.substr(assignment_start, assignment_end - assignment_start);
						}
					}

					// Remove "this." from the beginning
					reference = reference.substr(5);

					// How nested is it?
					std::vector<std::string> parts = explode(reference, '.');

					if (resolved.find(reference) == resolved.end()) {
						// Unresolved reference for this line, resolve it
						std::string initial_prepend = "local this__" + parts[0] + "=\"${__objectAddress}__" + parts[0] + "\"";
						result += initial_prepend + '\n';

						append += "unset this__" + parts[0] + '\n';

						for (size_t level = 1; level < parts.size(); level++) {
							// In a case such as '@this.member.submember.subsubmember' etc,
							// Each non-terminal part is a pointer which needs to be dereferenced before we can descend further to the next part
							// For example, in the above case, we need to do the following:
							// 1. Dereference 'member' (objectAddress__member) and get its address, call it 'member_address'.
							//		'submember' will then be found at 'member_address__submember'
							// 2. Dereference 'submember' (member_address__submember) and get its address, call it 'submember_address'
							//		 'subsubmember' will then be found at 'submember_address__subsubmember'
							// And so on...
							// We need to prepend the necessary code to the method body to handle this
							std::string parentvar_reference = "this";
							for (size_t i = 0; i < level; i++) {
								parentvar_reference += "__" + parts[i];
							}
							std::string prepend = "local " + parentvar_reference + "__" + parts[level]
								+ "=\"${!" + parentvar_reference + "}__" + parts[level] + "\"";

							append += "unset " + parentvar_reference + "__" + parts[level] + '\n';

							result += prepend + '\n';

						}
					}
					std::string written_reference = replace_all(reference, ".", "__");
					std::string new_line = "";
					if (!is_lvalue) {
						new_line = line.substr(0, match_start);
						new_line += "${!this__" + written_reference + "}";
						new_line += line.substr(match_end);
					} else {
						// Store the assignment into a variable before calling eval
						std::string assignment_variable = "this__" + written_reference + "____assignment";
						append += "unset " + assignment_variable + '\n';

						std::string assignment_line = "local " + assignment_variable + "=" + assignment + '\n';
						result += assignment_line;

						new_line = line.substr(0, match_start);
						new_line += "eval ${this__" + written_reference + "}=\\${" + assignment_variable + "}";
					}
					line = new_line;
				}

				resolved.insert(reference);
			}
			result += line + '\n';
			if (!append.empty()) {
				result += append;
			}

			previous_newline_escaped = line[line.length() - 1] == '\\';
		}
		return result;
	}
};

struct bpp_class {
	std::string name = "";
	std::map<std::string, std::string> data_members;
	std::vector<bpp_method> methods;

	std::string write_new() const {
		std::string result = replace_all(template_new_function, "%CLASS%", name);

		std::string assignments = "";
		for (const auto& member : data_members) {
			assignments += "	eval \"${__objectAddress}__" + member.first + "=\\\"" + member.second + "\\\"\"\n";
		}

		result = replace_all(result, "%ASSIGNMENTS%", assignments);

		return result;
	}

	std::string write_delete() const {
		std::string result = replace_all(template_delete_function, "%CLASS%", name);

		std::string deletions = "";
		for (const auto& member : data_members) {
			deletions += "	unset ${__objectAddress}__" + member.first + "\n";
		}
		result = replace_all(result, "%DELETIONS%", deletions);

		return result;
	}

	std::string write_copy() const {
		std::string result = replace_all(template_copy_function, "%CLASS%", name);

		std::string copies = "";
		for (const auto& member : data_members) {
			copies += "	local __copyFrom__" + member.first + "=\"${__copyFromAddress}__" + member.first + "\"\n";
			copies += "	eval \"${__copyToAddress}__" + member.first + "=\\${!__copyFrom__" + member.first + "}\"\n";
		}
		result = replace_all(result, "%COPIES%", copies);

		return result;
	}

	std::string write_default_toPrimitive() const {
		std::string result = "";

		result += "function bpp__" + name + "__toPrimitive() {\n";
		result += "	echo \"" + name + " Instance\"\n";
		result += "}\n";

		return result;
	}

	std::string write_methods() const {
		std::string result = "";
		for (const bpp_method& method : methods) {
			std::string this_method = replace_all(template_method, "%CLASS%", name);

			this_method = replace_all(this_method, "%SIGNATURE%", method.generate_signature());

			std::string params = "";
			int i = 3;
			for (const bpp_param& param : method.parameters) {
				params += param.name + "=\"$" + std::to_string(i) + "\" ";
				i++;
			}
			this_method = replace_all(this_method, "%PARAMS%", params);

			this_method = replace_all(this_method, "%BODY%", method.handle_method_body());
			result += this_method;
		}
		return result;
	}

	std::string write_class() const {
		std::string result = "";

		result += bpp_supershell_function;

		std::string class_base_functions = "";
		class_base_functions += write_new();
		class_base_functions += write_delete();
		class_base_functions += write_copy();
		class_base_functions += write_default_toPrimitive();
		class_base_functions += write_methods();

		result += class_base_functions;

		return result;
	}
};
