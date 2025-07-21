/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#include <fstream>

#include "../../antlr/BashppLexer.h"
#include "../../antlr/BashppParser.h"
#include "../../bpp_include/bpp_codegen.h"
#include "explode.h"

antlr4::tree::ParseTree* find_node_at_column(antlr4::tree::ParseTree* single_line_node, uint32_t column);

std::shared_ptr<bpp::bpp_entity> resolve_entity_at(
	const std::string& file,
	uint32_t line,
	uint32_t column,
	std::shared_ptr<bpp::bpp_program> program,
	const std::string& file_contents = ""
);
