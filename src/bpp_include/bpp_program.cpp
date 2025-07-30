/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "bpp.h"
#include "bpp_codegen.h"
#include "templates.h"

namespace bpp {

bpp_program::bpp_program() {
	primitive_class = std::make_shared<bpp_class>();
	primitive_class->set_name("primitive");
	classes["primitive"] = primitive_class;

	// Pre-allocate space for the unordered_maps:
	// classes, objects
	// This is a performance optimization
	classes.reserve(10);
	objects.reserve(20);
}

bool bpp_program::set_containing_class(std::weak_ptr<bpp_class> containing_class) {
	return false;
}

void bpp_program::set_output_stream(std::shared_ptr<std::ostream> output_stream) {
	code = output_stream;
}

std::shared_ptr<bpp::bpp_class> bpp_program::get_primitive_class() const {
	return primitive_class;
}

/**
 * @brief Prepare a class for addition to the program by adding it to the classes map
 * 
 * This means that the class will be accessible for use in the program,
 * but it does not add any code for the class yet.
 * 
 * That is handled by add_class().
 * 
 * @return true if the class was prepared successfully, false if the class already exists
 */
bool bpp_program::prepare_class(std::shared_ptr<bpp_class> class_) {
	std::string name = class_->get_name();
	if (classes.find(name) != classes.end()) {
		return false; // Class already exists
	}
	classes[name] = class_;
	return true;
}

/**
 * @brief Add a class to the program
 * 
 * This function adds a class to the program, including all necessary code for the class.
 */
bool bpp_program::add_class(std::shared_ptr<bpp_class> class_) {
	std::string name = class_->get_name();

	// Verify that the class has been prepared
	if (classes.find(name) == classes.end()) {
		return false;
	}

	// Add the code for the class
	std::string class_code = "";
	class_code += template_new_function;
	class_code += template_copy_function;

	// Each of these templates has placeholders that need to be replaced, in the format %PLACEHOLDER-NAME%

	// Replace all instances of %ASSIGNMENTS% with the assignments for the __new function
	std::string assignments = "";
	for (auto& dm : class_->get_datamembers()) {
		assignments += dm->get_pre_access_code() + "\n";
		if (dm->get_class()->get_name() == "primitive") {
			// Is it an array?
			if (dm->is_array()) {
				assignments += "	eval \"${__this}__" + dm->get_name() + "=" + dm->get_default_value() + "\"\n";
			} else {
				assignments += "	local __objAssignment=" + dm->get_default_value() + "\n";
				assignments += "	eval \"${__this}__" + dm->get_name() + "=\\$__objAssignment\"\n";
				assignments += "	unset __objAssignment\n";
			}
		} else if (dm->is_pointer()) {
			std::string default_value = dm->get_default_value();
			std::string default_value_preface = "";
			if (!default_value.empty() && default_value[0] == '$') {
				default_value_preface = "\\";
			}
			assignments += "	eval \"${__this}__" + dm->get_name() + "=" + default_value_preface + default_value + "\"\n";
		} else {
			// Call 'new' in a supershell and assign its output
			code_segment supershell_code = generate_supershell_code(
				"bpp__" + dm->get_class()->get_name() + "____new",
				false,
				nullptr,
				shared_from_this()
			);
			assignments += supershell_code.pre_code;
			assignments += "	eval \"${__this}__" + dm->get_name() + "=" + supershell_code.code + "\"\n";
			assignments += supershell_code.post_code;

			// Call the constructor if it exists
			if (dm->get_class()->get_method_UNSAFE("__constructor") != nullptr) {
				assignments += "	bpp__" + dm->get_class()->get_name() + "____constructor \"${__this}__" + dm->get_name() + "\"\n";
			}
		}
		assignments += dm->get_post_access_code() + "\n";
	}
	class_code = replace_all(class_code, "%ASSIGNMENTS%", assignments);
	assignments = "";

	// Replace all instances of %COPIES% with the copies for the __copy function
	std::string copies = "";
	for (auto& dm : class_->get_datamembers()) {
		copies += dm->get_pre_access_code() + "\n";
		if (dm->get_class()->get_name() == "primitive" || dm->is_pointer()) {
			copies += "	local __copyFrom__" + dm->get_name() + "=\"${__copyFromAddress}__" + dm->get_name() + "\"\n";
			copies += "	eval \"${__copyToAddress}__" + dm->get_name() + "=\\${!__copyFrom__" + dm->get_name() + "}\"\n";
		} else {
			copies += "	bpp__" + dm->get_class()->get_name() + "____copy ${__copyFromAddress}__" + dm->get_name() + " ${__copyToAddress}__" + dm->get_name() + "\n";
		}
		copies += dm->get_post_access_code() + "\n";
	}
	class_code = replace_all(class_code, "%COPIES%", copies);
	copies = "";

	// Replace all instances of %CLASS% with the class name
	class_code = replace_all(class_code, "%CLASS%", class_->get_name());

	// Declare the vTable
	std::string class_vTable = "declare -A bpp__" + name + "____vTable\n";

	// Add the class's parent to the vTable
	if (class_->get_parent() != nullptr) {
		class_vTable += "bpp__" + name + "____vTable[\"__parent__\"]=\"bpp__" + class_->get_parent()->get_name() + "____vTable\"\n";
	}

	// Add the methods
	for (auto& method : class_->get_methods()) {
		std::string method_code = template_method;
		std::string method_name = method->get_name();
		std::string params = method->get_parameters().empty() ? "" : "local ";
		for (size_t i = 0; i < method->get_parameters().size(); i++) {
			params += method->get_parameters()[i]->get_name() + "=\"$" + std::to_string(i + 1) + "\"";
			if (i < method->get_parameters().size() - 1) {
				params += " ";
			}
		}
		method_code = replace_all(method_code, "%CLASS%", class_->get_name());
		method_code = replace_all(method_code, "%SIGNATURE%", method_name);
		method_code = replace_all(method_code, "%PARAMS%", params);
		method_code = replace_all(method_code, "%METHODBODY%", method->get_code());
		class_code += method_code;

		// Add the method to the vTable if it is virtual
		if (method->is_virtual()) {
			class_vTable += "bpp__" + name + "____vTable[\"" + method_name + "\"]=\"bpp__" + name + "__" + method_name + "\"\n";
		}
	}

	*code << class_code << class_vTable << std::flush;

	return true;
}

void bpp_program::set_include_paths(std::shared_ptr<std::vector<std::string>> paths) {
	include_paths = paths;
}

std::shared_ptr<std::vector<std::string>> bpp_program::get_include_paths() const {
	return include_paths;
}

void bpp_program::increment_supershell_counter() {
	supershell_counter++;
	if (supershell_counter == 1) {
		// This is the first object to be created in a supershell
		// We need to add the code to create the supershell
		add_code_to_previous_line(bpp_supershell_function);
	}
}

uint64_t bpp_program::get_supershell_counter() const {
	return supershell_counter;
}

void bpp_program::increment_assignment_counter() {
	assignment_counter++;
}

uint64_t bpp_program::get_assignment_counter() const {
	return assignment_counter;
}

void bpp_program::increment_function_counter() {
	function_counter++;

	if (function_counter == 1) {
		// This is the first function called
		// Write the vTable_lookup code to the program
		add_code_to_previous_line(bpp_vtable_lookup);
	}
}

uint64_t bpp_program::get_function_counter() const {
	return function_counter;
}

void bpp_program::increment_dynamic_cast_counter() {
	dynamic_cast_counter++;

	if (dynamic_cast_counter == 1) {
		// This is the first dynamic_cast called
		// Write the dynamic_cast code to the program
		add_code_to_previous_line(bpp_dynamic_cast);
	}
}

uint64_t bpp_program::get_dynamic_cast_counter() const {
	return dynamic_cast_counter;
}

void bpp_program::increment_object_counter() {
	object_counter++;
}

uint64_t bpp_program::get_object_counter() const {
	return object_counter;
}

void bpp_program::set_target_bash_version(uint16_t major, uint16_t minor) {
	target_bash_version = {major, minor};
}

std::pair<uint16_t, uint16_t> bpp_program::get_target_bash_version() const {
	return target_bash_version;
}

void bpp_program::mark_entity(
	const std::string& file,
	uint32_t start_line, uint32_t start_column,
	uint32_t end_line, uint32_t end_column,
	std::shared_ptr<bpp::bpp_entity> entity
) {
	FilePosition start(start_line, start_column);
	FilePosition end(end_line, end_column);
	entity_maps[file].insert(start, end, entity); // RAII
}

std::shared_ptr<bpp::bpp_entity> bpp_program::get_active_entity(
	const std::string& file,
	uint32_t line, uint32_t column
) {
	auto it = entity_maps.find(file);
	if (it == entity_maps.end()) {
		return nullptr; // No entity map for this file
	}

	EntityMap& map = it->second;

	std::shared_ptr<bpp::bpp_entity> entity = map.find(FilePosition(line, column));
	if (entity == nullptr) {
		return shared_from_this(); // If no lower-level entity is found, return the program itself as the "active" entity at that point
	}
	return entity;
}

std::vector<std::string> bpp_program::get_source_files() const {
	std::vector<std::string> files;
	for (const auto& pair : entity_maps) {
		files.push_back(pair.first);
	}
	return files;
}

std::string bpp_program::get_main_source_file() const {
	return main_source_file;
}

void bpp_program::set_main_source_file(const std::string& file) {
	main_source_file = file;
}

void bpp_program::add_diagnostic(
	const std::string& file,
	diagnostic_type type,
	const std::string& message,
	uint32_t start_line, uint32_t start_column,
	uint32_t end_line, uint32_t end_column
) {
	bpp::diagnostic diag;
	diag.type = type;
	diag.message = message;
	diag.start_line = start_line;
	diag.start_column = start_column;
	diag.end_line = end_line;
	diag.end_column = end_column;
	diagnostics[file].push_back(diag);
}

std::vector<bpp::diagnostic> bpp_program::get_diagnostics(const std::string& file) const {
	auto it = diagnostics.find(file);
	if (it != diagnostics.end()) {
		return it->second;
	}
	return {}; // Return an empty vector if no diagnostics are found for the file
}

void bpp_program::clear_diagnostics(const std::string& file) {
	auto it = diagnostics.find(file);
	if (it != diagnostics.end()) {
		diagnostics.erase(it);
	}
}

} // namespace bpp
