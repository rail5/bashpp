/**
 * Bash++
 * Bash with classes
 * 
 * This file contains the definitions for the classes and objects used in the Bash++ compiler
 * 
 * Copyright (C) 2024 rail5
 */

#include <set>
#include <map>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <variant>

std::set<std::string> protected_keywords = {  "public", "private", "class", "method", "constructor", "primitive" };


class bpp_compiled_code;
class bpp_class;
class bpp_class_variable;
class bpp_class_method;
class bpp_object;

typedef
	std::variant<std::shared_ptr<bpp_class_variable>, std::shared_ptr<bpp_class_method>, std::shared_ptr<bpp_object>>
	bpp_reference;

class bpp_compiled_code {
	// This class represents the compiled code of a Bash++ program
	// It contains a sequential list of alternating std::strings and unresolved references
	// The std::strings are Bash code, and the unresolved references are references to objects or methods
	// It also contains a counter to keep track of the number of unresolved references
	// Before the code is written to a file, the unresolved references must be resolved

	// For example:
	// The line:
	//		echo @object.method
	// Would be represented as:
	//		{ "echo ", object.method }
	// And should be resolved to:
	//		echo $(resolved_object_method)
	// Before outputting
	// The resolved references should be stored in a map, with the reference as the key and the resolved value as the value
	// The resolved value should be a Bash expression
	private:
		std::vector<std::variant<std::string, bpp_reference>> code;
		int unresolved_references = 0;
		std::unordered_map<bpp_reference, std::string> resolved_references;

	public:
};

class bpp_class_variable {
	// A class variable is a variable declared within a class
	// It has a name, a type, and a scope
	// This is *not* the same as a bpp_object, which is an instance of a class
	// A class variable is a data member of a class
	// This class variable may be a primitive type, or a complex type (another class)
	// But the "class variable" is more like an object *template* than an object itself
	private:
		std::string variable_name;
		std::string variable_type;
		bool is_public;
		std::shared_ptr<bpp_object_scope> variable_scope;

	public:
		// Constructor
		bpp_class_variable(const std::string& name, const std::string& type, bool is_public, std::shared_ptr<bpp_object_scope> scope) 
			: variable_name(name), variable_type(type), is_public(is_public), variable_scope(scope) {}

		// Get the variable name
		std::string get_variable_name() {
			return variable_name;
		}

		// Get the variable type
		std::string get_variable_type() {
			return variable_type;
		}

		// Check if the variable is public
		bool get_is_public() {
			return is_public;
		}

		// Get the variable scope
		std::shared_ptr<bpp_object_scope> get_variable_scope() {
			return variable_scope;
		}
};

class bpp_class {
	// A class is a collection of variables, methods, and a constructor
	// It has a name, and a scope
	// The scope determines where the class is accessible from
	// The name is the class's identifier
	private:
		std::string class_name;
		std::shared_ptr<bpp_object_scope> class_scope;
		std::unordered_map<std::string, std::shared_ptr<bpp_class_variable>> variables;
		std::unordered_map<std::string, std::shared_ptr<bpp_class_method>> methods;
		std::shared_ptr<bpp_class_method> constructor;

	public:
		// Constructor
		bpp_class(const std::string& name, std::shared_ptr<bpp_object_scope> scope) : class_name(name), class_scope(scope) {}

		// Find a variable in the class
		std::shared_ptr<bpp_class_variable> find_variable(const std::string& name) {
			if (variables.find(name) != variables.end()) {
				return variables[name];
			} else {
				return nullptr;
			}
		}

		// Find a method in the class
		std::shared_ptr<bpp_class_method> find_method(const std::string& name) {
			if (methods.find(name) != methods.end()) {
				return methods[name];
			} else {
				return nullptr;
			}
		}

		// Get class name
		std::string get_class_name() {
			return class_name;
		}

		// Get class scope
		std::shared_ptr<bpp_object_scope> get_class_scope() {
			return class_scope;
		}

		// Get the constructor
		std::shared_ptr<bpp_class_method> get_constructor() {
			return constructor;
		}

		// Set the constructor
		void set_constructor(std::shared_ptr<bpp_class_method> constructor) {
			this->constructor = constructor;
		}

		// Add a variable to the class
		void add_variable(const std::string& name, std::shared_ptr<bpp_class_variable> variable) {
			// First, sanity checks

			// Make sure we don't already have a variable with this name
			if (variables.find(name) != variables.end()) {
				throw std::runtime_error("Variable '" + name + "' already defined in class '" + class_name + "'");
			}

			// Make sure we're not trying to add a variable with a reserved name
			if (protected_keywords.find(name) != protected_keywords.end()) {
				throw std::runtime_error("Cannot use keyword '" + name + "' as a variable name");
			}

			// Make sure the variable name isn't already being used as a class name
			//todo

			// Make sure the variable type is not the same as the containing class
			if (variable->get_variable_type() == class_name) {
				throw std::runtime_error("Cannot declare a variable of the same type as its containing class");
			}

			// Add the variable
			variables[name] = variable;
		}
};

struct bpp_object_scope : public std::enable_shared_from_this<bpp_object_scope> {
	// Conceptually, we have a tree of scopes. The root scope is global, and each class has its own scope.
	// If an object is declared in the global scope, it is accessible from anywhere in the program
	// If an object is declared within a class, it is accessible from anywhere within that class
	// If an object is declared within a method, it is accessible only within that method
	// Declaring in classes is a somewhat special case, for a few reasons:
	// 	1. We can run into a "nested" situation, where a class declares a member object of another class type
	// 		In this case, the object's public data members are accessible from the class which declared it
	//  2. Data members of a class can be declared @public or @private
	//		@public data members are accessible from anywhere in the program, provided the containing object is accessible
	//			They must also be referenced via the containing object (e.g, @object.dataMember)
	//		@private data members are only accessible from within the class which declared them
	//	When accessing a data member from *within* its containing class, the @object prefix is not required
	//
	// In general, when accessing an object, we must traverse the object tree to find the final object
	// This is because an object can be declared within another object, which can be declared within another object, and so on
	// We also need to consider method parameters, which are accessible only within the method in which they are declared
	// Finally, we need to consider global objects, which are accessible from anywhere in the program
	// All this is to say that we need to be able to determine the scope of an object at any point in the program

	std::string scope_name;
	std::unordered_map<std::string, std::shared_ptr<bpp_object>> objects;
	std::shared_ptr<bpp_object_scope> parent_scope;
	std::vector<std::shared_ptr<bpp_object_scope>> child_scopes;

	// Constructor
	bpp_object_scope(const std::string& name, std::shared_ptr<bpp_object_scope> parent_scope) : scope_name(name), parent_scope(parent_scope) {}

	void add_object(const std::string& object_name, std::shared_ptr<bpp_class> object_type, bool is_public) {
		objects[object_name] = std::make_shared<bpp_object>(bpp_object{ object_name, object_type, is_public });
	}

	std::shared_ptr<bpp_object_scope> add_child_scope(const std::string& name) {
		auto new_scope = std::make_shared<bpp_object_scope>(name, shared_from_this());
		child_scopes.push_back(new_scope);
		return new_scope;
	}

	// Find an object in the current scope or parent scopes
	std::shared_ptr<bpp_object> find_object(const std::string& name) {
		if (objects.find(name) != objects.end()) {
			return objects[name];
		} else if (parent_scope) {
			return parent_scope->find_object(name);
		} else {
			return nullptr;
		}
	}
};

class bpp_object : public std::enable_shared_from_this<bpp_object> {
	// An object is an instance of a class
	// It has a name, a type, and a scope
	// The scope determines where the object is accessible from
	// The type determines the object's data members and methods
	// The name is the object's identifier

	private:
		std::string object_name;
		std::shared_ptr<bpp_class> object_type;
		std::unordered_map<std::string, std::shared_ptr<bpp_object>> data_members;
		bool is_public;
		std::shared_ptr<bpp_object_scope> object_scope;
		std::shared_ptr<bpp_object> parent_object;

	public:
		// Constructor
		bpp_object(const std::string& name, std::shared_ptr<bpp_class> type, bool is_public) : object_name(name), object_type(type), is_public(is_public) {}

		// Find a data member of the object
		std::shared_ptr<bpp_object> find_data_member(const std::string& name) {
			if (data_members.find(name) != data_members.end()) {
				return data_members[name];
			} else {
				return nullptr;
			}
		}

		// Find a method of the object
		std::shared_ptr<bpp_class_method> find_method(const std::string& name) {
			return object_type->find_method(name);
		}

		// Instantiation of this object in a Bash++ program
		// We need to call its constructor (if present), as well as any constructors of its data members
		// The return value should be a std::string containing the Bash code to instantiate the object
		// Since Bash++ is a source-to-source compiler
		std::string instantiate() {
			std::string code;
			
			// Call the constructor
			if (object_type->get_constructor()) {
				code += object_type->get_constructor()->constructor_body;
			}

			for (auto& data_member : data_members) {
				code += data_member.second->instantiate();
			}
			return code;
		}
};

/*
class bpp_program {
	/**
	 * A program is a collection of classes, instantiated objects of those classes, and statements.
	 *//*
	private:
		std::set<std::string> symbols;
		std::map<std::string, bpp_class> classes;
		std::vector<std::string> statements;
		std::string source_file;
		std::map<std::string, int> line_numbers;
		std::shared_ptr<bpp_object_scope> global_scope;
	public:
		void add_statement(std::string statement) {
			statements.push_back(statement);
		}
		void add_class(bpp_class new_class) {
			classes[new_class.class_name] = new_class;
		}
		void add_object_to_scope(const std::string& scope_name, const std::string& object_name, std::shared_ptr<bpp_class> object_type, bool is_public) {
			auto scope = global_scope;
			if (scope_name != "global") {
				scope = global_scope->find_object(scope_name)->object_scope;
			}
		}
		std::string write_program();
};
*/