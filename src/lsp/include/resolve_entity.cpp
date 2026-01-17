/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "resolve_entity.h"
#include <memory>
#include <regex>
#include <unistd.h>

typedef void* yyscan_t;

struct LexerExtra;

extern int yylex_init(yyscan_t* scanner);
extern int yylex_destroy(yyscan_t scanner);
extern void yyset_in(FILE* in_str, yyscan_t scanner);

extern void initLexer(yyscan_t yyscanner);
extern void destroyLexer(yyscan_t yyscanner);

extern void set_utf16_mode(bool enable, yyscan_t yyscanner);

#include "../../flexbison/lex.yy.hpp"
#include "../../flexbison/parser.tab.hpp"

std::shared_ptr<AST::ASTNode> find_node_at_column(std::shared_ptr<AST::ASTNode> single_line_node, uint32_t column) {
	if (single_line_node == nullptr) return nullptr;

	uint64_t start_column = single_line_node->getCharPositionInLine();
	uint64_t stop_column = single_line_node->getEndPosition().column;

	if (column < start_column || column > stop_column) {
		return nullptr; // Column is outside the range of this node
	}

	for (size_t i = 0; i < single_line_node->getChildren().size(); i++) {
		const std::shared_ptr<AST::ASTNode> child = single_line_node->getChildren()[i];
		if (child) {
			std::shared_ptr<AST::ASTNode> result = find_node_at_column(child, column);
			if (result) {
				return result; // Found a child node that matches the column
			}
		}
	}
	return single_line_node;
}

std::shared_ptr<bpp::bpp_entity> resolve_entity_at(
	const std::string& file,
	uint32_t line,
	uint32_t column,
	std::shared_ptr<bpp::bpp_program> program,
	bool utf16_mode,
	const std::string& file_contents
) {
	if (program == nullptr || file.empty()) {
		return nullptr; // Invalid program or file
	}
	std::shared_ptr<bpp::bpp_entity> context = program->get_active_entity(file, line, column);
	if (context == nullptr) {
		context = program;
	}

	std::string line_content;
	// If we weren't provided with replacement contents,
	// Read from the file on the filesystem
	if (file_contents.empty()) {
		std::ifstream source_file(file);
		if (!source_file.is_open()) {
			return nullptr;
		}
		uint32_t current_line = 0;
		while (std::getline(source_file, line_content) && current_line < line) {
			current_line++;
		}
		source_file.close();

		if (current_line != line) {
			return nullptr; // Line does not exist
		}
	} else {
		// If replacement contents *were* provided,
		// Read from that instead
		std::istringstream stream(file_contents);
		uint32_t current_line = 0;
		while (std::getline(stream, line_content) && current_line < line) {
			current_line++;
		}

		if (current_line != line) {
			return nullptr; // Line does not exist
		}
	}

	// Run the parser on this one line
	try {
		const char* buffer = line_content.c_str();
		size_t size = line_content.size();
		FILE* line_stream = fmemopen(reinterpret_cast<void*>(const_cast<char*>(buffer)), size, "r");
		if (line_stream == nullptr) {
			return nullptr; // Could not open memory stream
		}

		yyscan_t lexer;
		if (yylex_init(&lexer) != 0) {
			fclose(line_stream);
			return nullptr; // Could not initialize lexer
		}
		yyset_in(line_stream, lexer);
		initLexer(lexer);
		::set_utf16_mode(utf16_mode, lexer);

		std::shared_ptr<AST::Program> astRoot = nullptr;
		yy::parser parser(astRoot, lexer);
		int parse_result = parser.parse();
		fclose(line_stream);
		destroyLexer(lexer);

		if (parse_result != 0 || astRoot == nullptr) {
			return nullptr; // Parsing failed
		}
		
		// Find the statement that intersects with the given column
		auto node = find_node_at_column(astRoot, column);
		if (!node) {
			return nullptr; // No node found at the specified column
		}

		bool is_object_reference = false;
		std::deque<std::string> identifiers;

		// First, check if it's an object reference
		if (node->getType() == AST::NodeType::ObjectReference) {
				is_object_reference = true;
				auto object_ref_ctx = std::dynamic_pointer_cast<AST::ObjectReference>(node);
				if (!object_ref_ctx) {
					return nullptr; // Not a valid object reference context
				}

				std::vector<AST::Token<std::string>> ids;
				ids.reserve(object_ref_ctx->IDENTIFIERS().size() + 1);
				ids.push_back(object_ref_ctx->IDENTIFIER());
				ids.insert(ids.end(), object_ref_ctx->IDENTIFIERS().begin(), object_ref_ctx->IDENTIFIERS().end());

				for (const auto& id : ids) {
					if (id.getCharPositionInLine() > column) {
						break;
					}
					identifiers.push_back(id.getValue());
				}
			// Resolve the object reference
			bpp::entity_reference entity_ref = bpp::resolve_reference(
				file,
				context,
				&identifiers,
				false,
				program
			);
			return entity_ref.entity;
		}
		
		// If we're here, it's not an object reference. Keep checking
		switch (node->getType()) {
			case AST::NodeType::IncludeStatement:
			{
				// Create a faux-entity that points to the included file line 0, column 0
				auto include_ctx = std::dynamic_pointer_cast<AST::IncludeStatement>(node);
				if (!include_ctx) {
					return nullptr; // Not a valid include statement context
				}
				bool local_include = include_ctx->PATHTYPE() == AST::IncludeStatement::PathType::QUOTED;
				std::string source_filename = include_ctx->PATH();

				// Resolve the path
				if (!local_include) {
					// Search for the file in the include paths
					bool found = false;
					for (const std::string& include_path : *program->get_include_paths()) {
						std::string full_path = include_path + "/" + source_filename;
						if (access(full_path.c_str(), F_OK) == 0) {
							source_filename = full_path;
							found = true;
							break;
						}
					}

					if (!found) {
						return nullptr; // File not found in include paths
					}
				} else {
					std::string current_directory;
					if (file == "<stdin>") {
						char current_working_directory[PATH_MAX];
						if (getcwd(current_working_directory, PATH_MAX) == nullptr) {
							throw internal_error("Could not get current working directory");
						}
						current_directory = current_working_directory;
					} else {
						current_directory = file.substr(0, file.find_last_of('/'));
					}

					// Append the source_filename to the directory
					source_filename = current_directory + "/" + source_filename;
				}

				// Get the full path of the file
				char full_path[PATH_MAX];
				if (realpath(source_filename.c_str(), full_path) == nullptr) {
					return nullptr; // File not found
				}

				std::shared_ptr<bpp::bpp_entity> faux_entity = std::make_shared<bpp::bpp_entity>();
				faux_entity->set_definition_position(source_filename, 0, 0);
				faux_entity->set_name(full_path);
				return faux_entity;
			}
			break;

			case AST::NodeType::ObjectInstantiation:
			{
				// Resolve either:
				// 1. The class being instantiated, or
				// 2. The object being instantiated
				// depending on the position of the column
				auto instantiation_ctx = std::dynamic_pointer_cast<AST::ObjectInstantiation>(node);
				if (!instantiation_ctx) {
					return nullptr; // Not a valid object instantiation context
				}

				auto class_name_token = instantiation_ctx->TYPE();
				
				auto object_name_token = instantiation_ctx->IDENTIFIER();
				
				// Are we being asked to resolve the class?
				uint64_t class_name_start = class_name_token.getCharPositionInLine();
				uint64_t class_name_end = class_name_start + class_name_token.getValue().length();
				if (column >= class_name_start && column <= class_name_end) {
					auto class_pointer = context->get_class(class_name_token.getValue());
					return class_pointer;
				}

				// Are we being asked to resolve the object?
				uint64_t object_name_start = object_name_token.getCharPositionInLine();
				uint64_t object_name_end = object_name_start + object_name_token.getValue().length();
				if (column >= object_name_start && column <= object_name_end) {
					// Resolve the object being instantiated
					std::shared_ptr<bpp::bpp_entity> object_pointer;
					// If we're inside a class, resolve the object as a data member
					// Otherwise, resolve it as an object
					std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(context);
					if (current_class) {
						object_pointer = current_class->get_datamember(object_name_token.getValue(), current_class);
					} else {
						object_pointer = context->get_object(object_name_token.getValue());
					}
					return object_pointer;
				}

				// If we reach here, the column is not within the class or object name tokens
				return nullptr;
			}
			break;

			case AST::NodeType::PointerDeclaration:
			{
				// Resolve either:
				// 1. The class type of the pointer, or
				// 2. The pointer variable itself
				// depending on the position of the column
				auto pointer_ctx = std::dynamic_pointer_cast<AST::PointerDeclaration>(node);
				if (!pointer_ctx) {
					return nullptr; // Not a valid pointer declaration context
				}

				auto type_token = pointer_ctx->TYPE();
				
				auto name_token = pointer_ctx->IDENTIFIER();
				
				// Are we being asked to resolve the class type?
				uint64_t type_start = type_token.getCharPositionInLine();
				uint64_t type_end = type_start + type_token.getValue().length();
				if (column >= type_start && column <= type_end) {
					auto class_pointer = context->get_class(type_token.getValue());
					return class_pointer;
				}

				// Are we being asked to resolve the pointer variable?
				uint64_t name_start = name_token.getCharPositionInLine();
				uint64_t name_end = name_start + name_token.getValue().length();
				if (column >= name_start && column <= name_end) {
					// Resolve the pointer variable
					std::shared_ptr<bpp::bpp_entity> pointer_variable;
					// If we're inside a class, resolve the pointer as a data member
					// Otherwise, resolve it as an object
					std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(context);
					if (current_class) {
						pointer_variable = current_class->get_datamember(name_token.getValue(), current_class);
					} else {
						pointer_variable = context->get_object(name_token.getValue());
					}
					return pointer_variable;
				}

				// If we reach here, the column is not within the class type or pointer variable tokens
				return nullptr;
			}
			break;

			case AST::NodeType::DatamemberDeclaration:
			{
				// If it's a non-primitive class member, this will be caught by object_instantiation or pointer_declaration
				// Here, it's a primitive member
				auto member_ctx = std::dynamic_pointer_cast<AST::DatamemberDeclaration>(node);
				if (!member_ctx) {
					return nullptr; // Not a valid data member declaration context
				}

				std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(context);
				if (!current_class) {
					return nullptr; // Not in a class context
				}

				std::shared_ptr<bpp::bpp_datamember> member = current_class->get_datamember(member_ctx->IDENTIFIER().value(), current_class);
				return member;
			}

			case AST::NodeType::NewStatement:
			{
				// Resolve the class being instantiated with 'new'
				auto new_ctx = std::dynamic_pointer_cast<AST::NewStatement>(node);
				if (!new_ctx) {
					return nullptr; // Not a valid new statement context
				}

				// Verify that the given position is within the class name token
				uint64_t class_name_start = new_ctx->TYPE().getCharPositionInLine();
				uint64_t class_name_end = class_name_start + new_ctx->TYPE().getValue().length();
				if (column < class_name_start || column > class_name_end) {
					return nullptr; // Column is outside the class name token
				}

				auto class_pointer = context->get_class(new_ctx->TYPE().getValue());
				return class_pointer;
			}
			break;

			case AST::NodeType::DynamicCastTarget:
			{
				// Resolve the class being cast to
				auto cast_ctx = std::dynamic_pointer_cast<AST::DynamicCastTarget>(node);
				if (!cast_ctx) {
					return nullptr; // Not a valid dynamic cast statement context
				}

				auto class_pointer = context->get_class(cast_ctx->TARGETTYPE().value());
				return class_pointer;
			}
			break;

			default:
				break;
		}
	} catch (...) {
		// Parser failed, ignore
	}

	// Nothing hit -- last ditch effort: try regex matching partial class or method definitions
	std::regex class_regex(R"(@class\s+([a-zA-Z_][a-zA-Z0-9_]*)(?:\s*:\s*([a-zA-Z_][a-zA-Z0-9_]*))?)");
	std::smatch class_match;
	if (std::regex_search(line_content, class_match, class_regex)) {
		
		std::string class_name = class_match[1].str();
		std::string parent_class_name = class_match[2].matched ? class_match[2].str() : "";

		auto class_name_start = class_match.position(1);
		auto class_name_end = class_name_start + class_match[1].length();

		auto parent_class_name_start = class_match[2].matched ? class_match.position(2) : -1;
		auto parent_class_name_end = parent_class_name_start + (parent_class_name.empty() ? 0 : class_match[2].length());

		if (column >= class_name_start && column <= class_name_end) {
			// We're resolving the class name
			auto class_pointer = program->get_class(class_name);
			return class_pointer;
		} else if (parent_class_name_start != -1 &&
		           column >= parent_class_name_start && column <= parent_class_name_end) {
			// We're resolving the parent class name
			auto parent_class_pointer = program->get_class(parent_class_name);
			return parent_class_pointer;
		}
	}

	std::regex method_regex(R"(@method\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*)");
	std::smatch method_match;
	if (std::regex_search(line_content, method_match, method_regex)) {
		std::string method_name = method_match[1].str();

		auto method_name_start = method_match.position(1);
		auto method_name_end = method_name_start + method_match[1].length();

		std::shared_ptr<bpp::bpp_method> method_ctx = std::dynamic_pointer_cast<bpp::bpp_method>(context);
		if (!method_ctx) {
			// Not in a method context, so we can't resolve the method
			return nullptr;
		}

		if (column >= method_name_start && column <= method_name_end) {
			return method_ctx;
		}
	}

	return nullptr;
}

std::string find_comments_for_entity(std::shared_ptr<bpp::bpp_entity> entity) {
	if (entity == nullptr) {
		return "";
	}

	// Locate the entity's definition position
	auto definition_position = entity->get_initial_definition();

	if (definition_position.file.empty()) {
		return ""; // No known source file, no alternative contents provided
	}

	// TODO(@rail5): Should we be concerned that we're reading from the filesystem here?
	// Should the language server keep a cache of file contents for parsed files?
	std::ifstream source_file(definition_position.file);
	if (!source_file.is_open()) {
		return ""; // Could not open the source file
	}

	std::string comments;
	std::string line;
	uint32_t current_line = 0;
	while (std::getline(source_file, line)) {
		if (current_line >= definition_position.line) {
			break; // Stop reading after reaching the definition line
		}
		current_line++;

		size_t comment_start = line.find('#');
		if (comment_start != std::string::npos) {
			// Extract the comment part
			std::string comment = line.substr(comment_start);
			// Trim leading whitespace from the comment
			comment.erase(0, comment.find_first_not_of(" \t"));

			if (!comments.empty()) {
				comments += "\n"; // Separate comments with a newline
			}
			comments += comment; // Append this comment to the comment block
		} else {
			comments.clear(); // A comment block ended, and we still haven't hit the definition line
				// In other words: the previous comment block is not associated with the entity
				// Therefore, clear it
		}
	}

	source_file.close();
	return comments;
}
