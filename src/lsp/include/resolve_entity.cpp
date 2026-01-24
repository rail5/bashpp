/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include "resolve_entity.h"
#include <memory>
#include <unistd.h>

#include "../../AST/BashppParser.h"

std::shared_ptr<AST::ASTNode> find_node_at_position(std::shared_ptr<AST::ASTNode> node, uint32_t line, uint32_t column) {
	if (node == nullptr) return nullptr;

	uint32_t start_line = node->getLine();
	uint32_t start_column = node->getCharPositionInLine();
	uint32_t stop_line = node->getEndPosition().line;
	uint32_t stop_column = node->getEndPosition().column;

	// Combine line and column into a single 64-bit value for easier comparison
	// The upper 32 bits are the line, the lower 32 bits are the column
	uint64_t start_pos = (static_cast<uint64_t>(start_line) << 32) | start_column;
	uint64_t stop_pos = (static_cast<uint64_t>(stop_line) << 32) | stop_column;

	uint64_t target_pos = (static_cast<uint64_t>(line) << 32) | column;

	if (target_pos < start_pos || target_pos > stop_pos) {
		return nullptr; // Position is outside the range of this node
	}

	for (size_t i = 0; i < node->getChildren().size(); i++) {
		const std::shared_ptr<AST::ASTNode>& child = node->getChildren()[i];
		if (child) {
			std::shared_ptr<AST::ASTNode> result = find_node_at_position(child, line, column);
			if (result) {
				return result; // Found a child node that matches the column
			}
		}
	}
	return node;
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

	AST::BashppParser parser;
	parser.setUTF16Mode(utf16_mode);
	if (file_contents.empty()) {
		// Parse the file on disk
		parser.setInputFromFilePath(file);
	} else {
		// Parse the provided file contents
		parser.setInputFromStringContents(file_contents);
	}

	auto astRoot = parser.program();
	if (!astRoot) return nullptr;

	auto node = find_node_at_position(astRoot, line, column);
	if (!node) return nullptr;

	uint64_t target_position = (static_cast<uint64_t>(line) << 32) | column;

	switch (node->getType()) {
		case AST::NodeType::ObjectReference:
			{
				auto object_ref_ctx = std::static_pointer_cast<AST::ObjectReference>(node);
				std::vector<std::string> identifiers;
				identifiers.push_back(object_ref_ctx->IDENTIFIER().getValue());
				for (const auto& id : object_ref_ctx->IDENTIFIERS()) {
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
			break;
		case AST::NodeType::IncludeStatement:
			{
				// Create a fake entity that points to the included file line 0, column 0
				auto include_ctx = std::static_pointer_cast<AST::IncludeStatement>(node);
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
				} else if (local_include) {
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
				auto faux_entity = std::make_shared<bpp::bpp_entity>();
				faux_entity->set_definition_position(source_filename, 1, 1);
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
				auto instantiation_ctx = std::static_pointer_cast<AST::ObjectInstantiation>(node);

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
					auto current_class = std::dynamic_pointer_cast<bpp::bpp_class>(context);
					if (current_class) {
						object_pointer = current_class->get_datamember(object_name_token.getValue(), current_class);
					} else {
						object_pointer = context->get_object(object_name_token.getValue());
					}
					return object_pointer;
				}
				return nullptr;
			}
			break;
		case AST::NodeType::PointerDeclaration:
			{
				// Resolve either:
				// 1. The class type of the pointer, or
				// 2. The pointer variable itself
				// depending on the position of the column
				auto pointer_ctx = std::static_pointer_cast<AST::PointerDeclaration>(node);

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
					auto current_class = std::dynamic_pointer_cast<bpp::bpp_class>(context);
					if (current_class) {
						pointer_variable = current_class->get_datamember(name_token.getValue(), current_class);
					} else {
						pointer_variable = context->get_object(name_token.getValue());
					}
					return pointer_variable;
				}
				return nullptr;
			}
			break;
		case AST::NodeType::DatamemberDeclaration:
			{
				// If it's a non-primitive class member, this will be caught by object_instantiation or pointer_declaration
				// Here, it's a primitive member
				auto member_ctx = std::static_pointer_cast<AST::DatamemberDeclaration>(node);

				auto current_class = std::dynamic_pointer_cast<bpp::bpp_class>(context);
				if (!current_class) return nullptr;
				auto member = current_class->get_datamember(member_ctx->IDENTIFIER().value(), current_class);
				return member;
			}
			break;
		case AST::NodeType::NewStatement:
			{
				// Resolve the class being instantiated with 'new'
				auto new_ctx = std::static_pointer_cast<AST::NewStatement>(node);

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
				auto cast_ctx = std::static_pointer_cast<AST::DynamicCastTarget>(node);

				auto target_type = cast_ctx->TARGETTYPE();
				if (!target_type.has_value()) return nullptr;
				std::string type_string = target_type.value().getValue();

				auto class_pointer = context->get_class(type_string);
				return class_pointer;
			}
			break;
		case AST::NodeType::ClassDefinition:
			{
				// Resolve either the class being defined or the parent class from which it inherits
				auto class_def_ctx = std::static_pointer_cast<AST::ClassDefinition>(node);
				auto class_name_token = class_def_ctx->CLASSNAME();
				auto parent_class_token = class_def_ctx->PARENTCLASSNAME();

				uint64_t class_name_start = (static_cast<uint64_t>(class_name_token.getLine()) << 32) | class_name_token.getCharPositionInLine();
				uint64_t class_name_end = class_name_start + class_name_token.getValue().length();

				uint64_t parent_class_start = 0;
				uint64_t parent_class_end = 0;

				if (parent_class_token.has_value()) {
					parent_class_start = (static_cast<uint64_t>(parent_class_token.value().getLine()) << 32) | parent_class_token.value().getCharPositionInLine();
					parent_class_end = parent_class_start + parent_class_token.value().getValue().length();
				}

				// Are we being asked to resolve the class being defined?
				if (target_position >= class_name_start && target_position <= class_name_end) {
					auto class_pointer = program->get_class(class_name_token.getValue());
					return class_pointer;
				}

				// Are we being asked to resolve the parent class?
				if (parent_class_token.has_value() && target_position >= parent_class_start && target_position <= parent_class_end) {
					auto class_pointer = program->get_class(parent_class_token.value().getValue());
					return class_pointer;
				}

				return nullptr;
			}
			break;
		case AST::NodeType::MethodDefinition:
			{
				// Resolve either:
				// 1. The method being defined
				// 2. One of the method's parameters
				// 3. The type of one of the method's parameters
				std::cerr << "hit" << std::endl;
				auto current_class = context->get_containing_class().lock();
				if (!current_class) return nullptr;
				auto method_def_ctx = std::static_pointer_cast<AST::MethodDefinition>(node);
				auto method_name_token = method_def_ctx->NAME();
				auto parameters = method_def_ctx->PARAMETERS();

				uint64_t method_name_start = (static_cast<uint64_t>(method_name_token.getLine()) << 32) | method_name_token.getCharPositionInLine();
				uint64_t method_name_end = method_name_start + method_name_token.getValue().length();

				if (target_position >= method_name_start && target_position <= method_name_end) {
					// Resolve the method being defined
					auto method = current_class->get_method_UNSAFE(method_name_token.getValue());
					return method;
				}

				for (const auto& param : parameters) {
					uint64_t param_start = (static_cast<uint64_t>(param.getLine()) << 32) | param.getCharPositionInLine();

					std::string param_string = "";
					if (param.getValue().type.has_value()) {
						param_string += "@" + param.getValue().type.value().getValue();
						if (param.getValue().pointer) {
							param_string += "*";
						}
						param_string += " ";
					}
					param_string += param.getValue().name.getValue();

					uint64_t param_end = param_start + param_string.length();
					if (target_position < param_start || target_position > param_end) {
						continue;
					}

					auto param_name_token = param.getValue().name;
					auto param_type_token_opt = param.getValue().type;

					uint64_t param_name_start = (static_cast<uint64_t>(param_name_token.getLine()) << 32) | param_name_token.getCharPositionInLine();
					uint64_t param_name_end = param_name_start + param_name_token.getValue().length();

					if (target_position >= param_name_start && target_position <= param_name_end) {
						// Resolve the parameter variable
						auto method = current_class->get_method_UNSAFE(method_name_token.getValue());
						if (!method) return nullptr;
						auto obj = method->get_object(param_name_token.getValue());
						return obj;
					}

					if (!param_type_token_opt.has_value()) continue;
					auto param_type_token = param_type_token_opt.value();
					uint64_t param_type_start = (static_cast<uint64_t>(param_type_token.getLine()) << 32) | param_type_token.getCharPositionInLine();
					uint64_t param_type_end = param_type_start + param_type_token.getValue().length();
					if (target_position >= param_type_start && target_position <= param_type_end) {
						// Resolve the parameter type
						auto method = current_class->get_method_UNSAFE(method_name_token.getValue());
						if (!method) return nullptr;
						for (const auto& method_param : method->get_parameters()) {
							if (method_param->get_name() == param_name_token.getValue()) {
								return method_param->get_class();
							}
						}
					}
				}
				return nullptr;
			}
			break;
		default:
			// DEBUG: REMOVE LATER
			std::cerr << "Innermost node type is " << static_cast<int>(node->getType()) << std::endl;
			return nullptr; // Not a resolvable node type
	}
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
	uint32_t current_line = 1;
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
