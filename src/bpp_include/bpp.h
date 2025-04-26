/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BPP_H_
#define SRC_BPP_INCLUDE_BPP_H_

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <memory>

#include "replace_all.cpp"

namespace bpp {

enum bpp_scope {
	SCOPE_PUBLIC,
	SCOPE_PROTECTED,
	SCOPE_PRIVATE,
	SCOPE_INACCESSIBLE
};

enum reference_type {
	ref_primitive,
	ref_method,
	ref_object
};

// Forward declarations

class bpp_entity;
class bpp_code_entity;
class bpp_string;
class bpp_method;
class bpp_constructor;
class bpp_destructor;
class bpp_method_parameter;
class bpp_class;
class bpp_object;
class bpp_datamember;
class bpp_program;

// Statement types
class bash_while_loop;
class bash_while_condition;
class bash_if;
class bash_if_branch;
class bash_case;
class bash_case_pattern;
class bash_for;
class bpp_delete_statement;
class bpp_dynamic_cast_statement;
class bpp_pointer_dereference;
class bpp_value_assignment;
class bpp_object_assignment;
class bpp_object_reference;
class bpp_object_address;

/**
 * @var inaccessible_entity
 * @brief A placeholder for an inaccessible entity (scope handling)
 */
static const std::shared_ptr<bpp_entity> inaccessible_entity = std::make_shared<bpp_entity>();

/**
 * @var inaccessible_datamember
 * @brief A placeholder for an inaccessible data member of a class (scope handling)
 */
static const std::shared_ptr<bpp_datamember> inaccessible_datamember = std::make_shared<bpp_datamember>();

/**
 * @var inaccessible_method
 * @brief A placeholder for an inaccessible method of a class (scope handling)
 */
static const std::shared_ptr<bpp_method> inaccessible_method = std::make_shared<bpp_method>();

/**
 * @var bpp_nullptr
 * @brief The secret internal value of '@nullptr' in Bash++
 */
static const char bpp_nullptr[] = "0";

/**
 * @class bpp_entity
 * @brief The base class for all entities in the Bash++ compiler
 * 
 * An entity is a class, object, method, or other construct in the Bash++ compiler.
 * This class provides the basic functionality for all entities.
 */
class bpp_entity {
	protected:
		/**
		 * @var classes
		 * @brief A map of class names to class objects within this entity
		 */
		std::unordered_map<std::string, std::shared_ptr<bpp_class>> classes;

		/**
		 * @var objects
		 * @brief A map of object names to bpp_objects within this entity
		 */
		std::unordered_map<std::string, std::shared_ptr<bpp_object>> objects;

		/**
		 * @var local_objects
		 * @brief Like objects, but only for objects whose scope is local to this entity
		 */
		std::unordered_map<std::string, std::shared_ptr<bpp_object>> local_objects;
		std::shared_ptr<bpp_class> type = nullptr;
		std::weak_ptr<bpp_class> containing_class;
		std::vector<std::shared_ptr<bpp_class>> parents;
	public:
		virtual ~bpp_entity() = default;
		virtual bool add_class(std::shared_ptr<bpp_class> class_);
		virtual bool add_object(std::shared_ptr<bpp_object> object, bool make_local = false);

		virtual std::shared_ptr<bpp_class> get_class() const;
		virtual std::string get_address() const;
		virtual std::string get_name() const;
		virtual std::weak_ptr<bpp::bpp_class> get_containing_class() const;
		virtual bool set_containing_class(std::weak_ptr<bpp::bpp_class> containing_class);

		virtual void inherit(std::shared_ptr<bpp_entity> parent);
		virtual void inherit(std::shared_ptr<bpp_class> parent);

		virtual std::unordered_map<std::string, std::shared_ptr<bpp_class>> get_classes() const;
		virtual std::unordered_map<std::string, std::shared_ptr<bpp_object>> get_objects() const;
		virtual std::shared_ptr<bpp_class> get_class(const std::string& name);
		virtual std::shared_ptr<bpp_object> get_object(const std::string& name);
};

/**
 * @class bpp_code_entity
 * @brief An entity which can contain code
 * 
 * Such as a method, a supershell, or the program itself
 * 
 * This class provides the basic functionality for entities which can contain code
 * Including 3 distinct code buffers:
 * - pre_code: Code that should be executed before the main code
 * - code: The main code
 * - post_code: Code that should be executed after the main code
 * 
 * Generally, the pre_code and post_code are used to set up and clean up the environment
 * 
 * This class also provides the ability to add code to the pre_code, code, and post_code buffers
 * And to flush those buffers when necessary
 */
class bpp_code_entity : public bpp_entity {
	protected:
		std::shared_ptr<std::ostream> code = std::make_shared<std::ostringstream>();
		std::string nextline_buffer = "";
		std::string postline_buffer = "";
		bool buffers_flushed = false;
	public:
		bpp_code_entity();
		virtual ~bpp_code_entity() = default;

		virtual void add_code(const std::string& code, bool add_newline = true);
		virtual void add_code_to_previous_line(const std::string& code);
		virtual void add_code_to_next_line(const std::string& code);

		bool add_object(std::shared_ptr<bpp_object> object, bool make_local = false) override;

		virtual void flush_nextline_buffer();
		virtual void flush_postline_buffer();
		virtual void flush_code_buffers();

		virtual void clear_all_buffers();

		virtual std::string get_code() const;
		virtual std::string get_pre_code() const;
		virtual std::string get_post_code() const;
};

/**
* @class bpp_string
* 
* @brief The practical difference between bpp_code_entity and bpp_string is how we handle the code buffers.
*
* When you call add_code() on a bpp_code_entity, it will flush the pre_code and post_code buffers.
*
* Meaning that we're treating the pre_code and post_code buffers as the pre- and post-code for *this particular line* of code.
* So when we finish writing this line of code, we should ensure that this line of code is preceded by its pre-code, and followed by its post-code
* 
* In a bpp_code_entity, the code:
*
* 	echo @this.dataMember
*
* Becomes:
*
* 	{pre-code necessary to fetch @this.dataMember}
* 	echo ${the resolved reference}
* 	{post-code necessary to clear the memory}
* 
* In a bpp_string, however, calling add_code() does not ever flush those buffers.
* 	Why?
*
* Because, if you imagine we're literally dealing with an actual string (although the same rules apply in some other cases),
* 	it's important not to muddy up the contents of the string with a bunch of pre- and post-code.
* 	If, for example, there are object references within the string, they should be resolved BEFORE the entire string,
* 	And cleared AFTER the entire string
* 
* This ensures that for a bpp_string, the code:
*
* 	echo "This is a very long string
* 		That spans multiple lines
* 		And has a reference to @this.dataMember"
*
* Becomes:
*
* 	{pre-code necessary to fetch @this.dataMember}
* 	echo "This is a very long string
* 		That spans multiple lines
* 		And has a reference to ${the resolved reference}"
* 	{post-code necessary to clear the memory}
* 
* So, bpp_string gives us more direct control over where the pre- and post-code is added
* 
* In some other part of the compiler where we're parsing the above example string,
* the code might look something like:
*
* 	entity->add_code_to_previous_line(pre_code);
* 	entity->add_code_to_next_line(post_code);
* 		^ These two lines prepare the code buffers in the code_entity
* 	entity->add_code("echo \"This is a very long string....."); // etc
* 		^ This line adds the code & *maybe* flushes the buffers we just prepared
*
* Where "entity" may be a bpp_code_entity or a bpp_string
* 
* In general, parser rules which handle things such as object references
* 	don't pay attention to whether they're dealing with a bpp_code_entity or a bpp_string.
* The code necessary to fetch an object reference (for example) is simply added to the pre-code buffer,
* 	and the code necessary to clear that memory is simply added to the post-code buffer.
* The question of whether or when those buffers should be flushed is left to the entity itself (code_entity or string).
*/
class bpp_string : public bpp_code_entity {
	public:
		bpp_string();

		void add_code(const std::string& code, bool add_newline = true) override;
		void add_code_to_previous_line(const std::string& code) override;
		void add_code_to_next_line(const std::string& code) override;

		std::string get_code() const override;
		std::string get_pre_code() const override;
		std::string get_post_code() const override;
};

/**
 * @class bpp_method
 * 
 * @brief A method in a class
 */
class bpp_method : public bpp_code_entity {
	private:
		std::string name;
		std::vector<std::shared_ptr<bpp_method_parameter>> parameters;
		bpp_scope scope = SCOPE_PRIVATE;
		bool m_is_virtual = false;
		bool inherited = false;
		bool add_object_as_parameter(std::shared_ptr<bpp_object> object);
	public:
		bpp_method();
		explicit bpp_method(const std::string& name);

		virtual bool add_parameter(std::shared_ptr<bpp_method_parameter> parameter);
		void set_name(const std::string& name);
		void set_scope(bpp_scope scope);
		void set_virtual(bool is_virtual);
		void set_inherited(bool is_inherited);

		std::string get_name() const;
		std::vector<std::shared_ptr<bpp_method_parameter>> get_parameters() const;
		bpp_scope get_scope() const;
		bool is_virtual() const;
		bool is_inherited() const;

		void destruct_local_objects();
};

/**
 * @class bpp_constructor
 * @brief A constructor in a class
 */
class bpp_constructor : public bpp_method {
	public:
		bpp_constructor();

		bool add_parameter(std::shared_ptr<bpp_method_parameter> parameter) override;
};

/**
 * @class bpp_destructor
 * @brief A destructor in a class
 */
class bpp_destructor : public bpp_method {
	public:
		bpp_destructor();

		bool add_parameter(std::shared_ptr<bpp_method_parameter> parameter) override;
};

/**
 * @class bpp_method_parameter
 * 
 * @brief A parameter in a method
 */
class bpp_method_parameter : public bpp_entity {
	private:
		std::shared_ptr<bpp_class> type;
		std::string name;
	public:
		explicit bpp_method_parameter(const std::string& name);

		void set_type(std::shared_ptr<bpp_class>);

		std::string get_name() const;
		std::shared_ptr<bpp_class> get_type() const;
};

/**
 * @class bpp_class
 * 
 * @brief A class in Bash++
 */
class bpp_class : public bpp_entity, public std::enable_shared_from_this<bpp_class> {
	private:
		std::string name;
		std::vector<std::shared_ptr<bpp_method>> methods;
		std::vector<std::shared_ptr<bpp_datamember>> datamembers;
		std::shared_ptr<bpp_constructor> constructor;
		std::shared_ptr<bpp_destructor> destructor;
		bool constructor_set = false;
		bool destructor_set = false;
		bool has_custom_toPrimitive = false;

		void remove_default_toPrimitive();
		void add_default_toPrimitive();
	public:
		bpp_class();

		std::weak_ptr<bpp_class> get_containing_class() const override;
		bool set_containing_class(std::weak_ptr<bpp::bpp_class> containing_class) override;

		std::shared_ptr<bpp_class> get_class() const override;

		void set_name(const std::string& name);
		bool add_method(std::shared_ptr<bpp_method> method);
		bool add_datamember(std::shared_ptr<bpp_datamember> datamember);
		bool set_constructor(std::shared_ptr<bpp_constructor> constructor);
		bool set_destructor(std::shared_ptr<bpp_destructor> destructor);

		std::string get_name() const;
		std::vector<std::shared_ptr<bpp_method>> get_methods() const;
		std::vector<std::shared_ptr<bpp_datamember>> get_datamembers() const;
		std::shared_ptr<bpp_constructor> get_constructor() const;
		std::shared_ptr<bpp_destructor> get_destructor() const;
		bool has_constructor() const;
		bool has_destructor() const;

		std::shared_ptr<bpp_method> get_method(const std::string& name, std::shared_ptr<bpp_entity> context);
		std::shared_ptr<bpp_method> get_method_UNSAFE(const std::string& name);
		std::shared_ptr<bpp_datamember> get_datamember(const std::string& name, std::shared_ptr<bpp_entity> context);

		void inherit(std::shared_ptr<bpp_class> parent) override;
		std::shared_ptr<bpp::bpp_class> get_parent();
};

/**
 * @class bpp_object
 * 
 * @brief An object in Bash++
 */
class bpp_object : public bpp_entity {
	protected:
		std::string name = "";
		std::string address = "";
		std::string assignment_value = "";
		std::string pre_access_code = "";
		std::string post_access_code = "";
		std::shared_ptr<bpp_class> type;
		bool m_is_pointer = false;
		std::shared_ptr<bpp::bpp_object> copy_from = nullptr;
	public:
		bpp_object();
		explicit bpp_object(const std::string& name);
		bpp_object(const std::string& name, bool is_pointer);

		void set_class(std::shared_ptr<bpp_class> object_class);
		void set_pointer(bool is_pointer);
		void set_name(const std::string& name);
		void set_address(const std::string& address);
		void set_assignment_value(const std::string& assignment_value);
		void set_pre_access_code(const std::string& pre_access_code);
		void set_post_access_code(const std::string& post_access_code);
		void set_nullptr();

		std::string get_name() const;
		virtual std::string get_address() const;
		std::string get_assignment_value() const;
		std::shared_ptr<bpp_class> get_class() const;
		std::string get_pre_access_code() const;
		std::string get_post_access_code() const;
		std::shared_ptr<bpp::bpp_object> get_copy_from() const;

		bool is_pointer() const;
};

/**
 * @class bpp_datamember
 * 
 * @brief A data member in a class
 */
class bpp_datamember : public bpp_object {
	private:
		std::string default_value = "";
		bpp_scope scope = SCOPE_PRIVATE;
		bool array = false;
	public:
		bpp_datamember();

		void set_default_value(const std::string& default_value);
		void set_scope(bpp_scope scope);
		void set_array(bool is_array);

		std::string get_address() const override;
		std::string get_default_value() const;
		bpp_scope get_scope() const;
		bool is_array() const;
};

/**
 * @class bpp_program
 * 
 * @brief The main program
 */
class bpp_program : public bpp_code_entity {
	private:
		std::shared_ptr<bpp_class> primitive_class;
		uint64_t supershell_counter = 0;
		uint64_t assignment_counter = 0;
		uint64_t function_counter = 0;
		uint64_t dynamic_cast_counter = 0;
		uint64_t object_counter = 0;
	public:
		bpp_program();

		bool set_containing_class(std::weak_ptr<bpp_class> containing_class) override;
		void set_output_stream(std::shared_ptr<std::ostream> output_stream);

		bool add_class(std::shared_ptr<bpp_class> class_) override;

		std::shared_ptr<bpp_class> get_primitive_class() const;

		void increment_supershell_counter();
		uint64_t get_supershell_counter() const;

		void increment_assignment_counter();
		uint64_t get_assignment_counter() const;

		void increment_function_counter();
		uint64_t get_function_counter() const;

		void increment_dynamic_cast_counter();
		uint64_t get_dynamic_cast_counter() const;

		void increment_object_counter();
		uint64_t get_object_counter() const;
};

/**
 * @class bash_while_loop
 * 
 * @brief A while loop in Bash++
 * 
 * This entity gets pushed onto the entity stack when a while loop is encountered in Bash++ code.
 * It contains a bash_while_condition object which holds the condition for the while loop
 * 
 * The reason for this is that the condition for the while loop may contain references which need to be resolved
 * And the pre- and post-code for those references need to be added in specific places in the compiled code.
 * E.g., supershells must be re-evaluated for each iteration of the loop
 * 
 * The 'bash_' prefix signifies that this is used to parse ordinary Bash code, not anything specific to Bash++
 */
class bash_while_loop : public bpp_code_entity {
	private:
		std::shared_ptr<bpp::bash_while_condition> while_condition;
	public:
		bash_while_loop();

		void set_while_condition(std::shared_ptr<bpp::bash_while_condition> while_condition);
		std::shared_ptr<bpp::bash_while_condition> get_while_condition() const;

		std::string get_code() const override;
		std::string get_pre_code() const override;
		std::string get_post_code() const override;
};

/**
 * @class bash_while_condition
 * 
 * @brief The condition for a while loop in Bash++
 * 
 * The 'bash_' prefix signifies that this is used to parse ordinary Bash code, not anything specific to Bash++
 */
class bash_while_condition : public bpp_string {
	private:
		int supershell_count = 0;
		std::vector<std::string> supershell_function_calls = {};
	public:
		bash_while_condition();

		void increment_supershell_count();
		void add_supershell_function_call(const std::string& function_call);
		std::vector<std::string> get_supershell_function_calls() const;
};

/**
 * @class bash_if
 * 
 * @brief An if statement in Bash++
 * 
 * This entity gets pushed onto the entity stack when an if statement is encountered in Bash++ code.
 * It contains a vector of conditional branches, each of which contains a condition and a branch of code
 * 
 * The reason this requires its own entity type is similar to the reason for bash_while_loop:
 * The conditions for the if statement may contain references which need to be resolved,
 * And the pre- and post-code for those references need to be added in specific places in the compiled code.
 * 
 * In the case of 'if' statements, the pre- and post-code is added before and after the entire if statement.
 * If these were parsed without their own entity type (e.g., just using a bpp_code_entity), the pre- and post-code
 * would be added before and after each individual conditional branch, which is incorrect.
 * 
 * The 'bash_' prefix signifies that this is used to parse ordinary Bash code, not anything specific to Bash++
 */
class bash_if : public bpp_string {
	private:
		std::string conditional_branch_pre_code = "";
		std::string conditional_branch_post_code = "";
		std::vector<std::pair<std::string, std::string>> conditional_branches = {};
	public:
		bash_if();

		void add_conditional_branch_pre_code(const std::string& pre_code);
		void add_conditional_branch_post_code(const std::string& post_code);
		void new_branch();
		void add_condition_code(const std::string& condition_code);
		void add_branch_code(const std::string& branch_code);
		std::string get_conditional_branch_pre_code() const;
		std::string get_conditional_branch_post_code() const;
		const std::vector<std::pair<std::string, std::string>>& get_conditional_branches() const;
};

/**
 * @class bash_if_branch
 * 
 * @brief A branch of an if statement in Bash++
 * 
 * This entity contains the *code* which is executed for a given branch of an if statement.
 * It is not responsible for parsing the condition of the if statement.
 * 
 * The 'bash_' prefix signifies that this is used to parse ordinary Bash code, not anything specific to Bash++
 */
class bash_if_branch : public bpp_code_entity {
	private:
		std::shared_ptr<bpp::bash_if> if_statement;
	public:
		bash_if_branch();

		void set_if_statement(std::shared_ptr<bpp::bash_if> if_statement);
		std::shared_ptr<bpp::bash_if> get_if_statement() const;

		std::string get_code() const override;
		std::string get_pre_code() const override;
		std::string get_post_code() const override;
};

/**
 * @class bash_case
 * 
 * @brief A case statement in Bash++
 * 
 * This entity gets pushed onto the entity stack when a case statement is encountered in Bash++ code.
 * 
 * The 'bash_' prefix signifies that this is used to parse ordinary Bash code, not anything specific to Bash++
 */
class bash_case : public bpp_string {
	private:
		std::string cases = "";
	public:
		bash_case();

		void add_case(const std::string& case_);

		const std::string& get_cases() const;
};

/**
 * @class bash_case_pattern
 * 
 * @brief A pattern for a case statement in Bash++
 * 
 * This entity contains a pattern to be matched in a case statement.
 * 
 * The 'bash_' prefix signifies that this is used to parse ordinary Bash code, not anything specific to Bash++
 */
class bash_case_pattern : public bpp_code_entity {
	private:
		std::string pattern = "";
		std::shared_ptr<bpp::bash_case> containing_case;
	public:
		bash_case_pattern();

		void set_pattern(const std::string& pattern);
		void set_containing_case(std::shared_ptr<bpp::bash_case> containing_case);

		const std::string& get_pattern() const;
		std::shared_ptr<bpp::bash_case> get_containing_case() const;
};

/**
 * @class bash_for
 * 
 * @brief A for loop in Bash++
 * 
 * This entity gets pushed onto the entity stack when a for loop is encountered in Bash++ code.
 * 
 * The 'bash_' prefix signifies that this is used to parse ordinary Bash code, not anything specific to Bash++
 */
class bash_for : public bpp_code_entity {
	private:
		std::string header_pre_code = "";
		std::string header_post_code = "";
		std::string header_code = "";
	public:
		bash_for();

		void set_header_pre_code(const std::string& pre_code);
		void set_header_post_code(const std::string& post_code);
		void set_header_code(const std::string& code);

		const std::string& get_header_pre_code() const;
		const std::string& get_header_post_code() const;
		const std::string& get_header_code() const;
};

/**
 * @class bpp_delete_statement
 * 
 * @brief A delete statement in Bash++
 * 
 * This entity gets pushed onto the entity stack when we encounter a '@delete' statement in Bash++ code.
 * 
 * It contains a pointer to the object that we intend to delete
 * 
 * The 'bpp_' prefix signifies that this is used to parse a statement type which is unique to Bash++
 */
class bpp_delete_statement : public bpp_string {
	private:
		std::shared_ptr<bpp::bpp_object> object_to_delete;
		bool force_ptr = false;
	public:
		bpp_delete_statement() = default;

		void set_object_to_delete(std::shared_ptr<bpp::bpp_object> object);
		void set_force_pointer(bool force_pointer);
		std::shared_ptr<bpp::bpp_object> get_object_to_delete() const;
		bool force_pointer() const;
};

/**
 * @class bpp_dynamic_cast_statement
 * 
 * @brief A dynamic_cast statement in Bash++
 * 
 * This entity gets pushed onto the entity stack when we encounter a `@dynamic_cast` statement in Bash++ code.
 * 
 * It contains a pointer to the class we're casting to, and the address we're casting
 * 
 * The 'bpp_' prefix signifies that this is used to parse a statement type which is unique to Bash++
 */
class bpp_dynamic_cast_statement : public bpp_string {
	private:
		std::shared_ptr<bpp::bpp_class> cast_to;
		std::string cast_address = "";
	public:
		bpp_dynamic_cast_statement();

		void set_cast_to(std::shared_ptr<bpp::bpp_class> cast_to);
		void set_cast_address(const std::string& cast_address);
		std::shared_ptr<bpp::bpp_class> get_cast_to() const;
		std::string get_cast_address() const;
};

/**
 * @class bpp_pointer_dereference
 * 
 * @brief A pointer dereference in Bash++
 * 
 * The 'bpp_' prefix signifies that this is used to parse a statement type which is unique to Bash++
 */
class bpp_pointer_dereference : public bpp_string {
	private:
		std::shared_ptr<bpp::bpp_value_assignment> value_assignment;
	public:
		bpp_pointer_dereference();

		void set_value_assignment(std::shared_ptr<bpp::bpp_value_assignment> value_assignment);
		std::shared_ptr<bpp::bpp_value_assignment> get_value_assignment() const;
};

/**
 * @class bpp_value_assignment
 * 
 * @brief A value assignment statement in Bash++
 * 
 * The 'bpp_' prefix signifies that this is used to parse a statement type which is unique to Bash++
 */
class bpp_value_assignment : public bpp_string {
	private:
		bool nonprimitive_assignment = false;
		std::shared_ptr<bpp_entity> nonprimitive_object;
		bool lvalue_nonprimitive = false;
		bool array_assignment = false;
		bool adding = false;
	public:
		bpp_value_assignment();

		void set_nonprimitive_assignment(bool is_nonprimitive);
		void set_nonprimitive_object(std::shared_ptr<bpp_entity> object);
		void set_lvalue_nonprimitive(bool is_nonprimitive);
		void set_array_assignment(bool is_array);
		void set_adding(bool is_adding);

		bool is_nonprimitive_assignment() const;
		std::shared_ptr<bpp_entity> get_nonprimitive_object() const;
		bool lvalue_is_nonprimitive() const;
		bool is_array_assignment() const;
		bool is_adding() const;
};

/**
 * @class bpp_object_assignment
 * 
 * @brief An object assignment statement in Bash++
 * 
 * The 'bpp_' prefix signifies that this is used to parse a statement type which is unique to Bash++
 */
class bpp_object_assignment : public bpp_string {
	private:
		std::string lvalue = "";
		std::string rvalue = "";
		bool lvalue_nonprimitive = false;
		bool rvalue_nonprimitive = false;
		std::shared_ptr<bpp_entity> lvalue_object;
		std::shared_ptr<bpp_entity> rvalue_object;
		bool adding = false;
		bool rvalue_array = false;
	public:
		bpp_object_assignment();

		void set_lvalue(const std::string& lvalue);
		void set_rvalue(const std::string& rvalue);
		void set_lvalue_nonprimitive(bool is_nonprimitive);
		void set_rvalue_nonprimitive(bool is_nonprimitive);
		void set_lvalue_object(std::shared_ptr<bpp_entity> object);
		void set_rvalue_object(std::shared_ptr<bpp_entity> object);
		void set_adding(bool is_adding);
		void set_rvalue_array(bool is_array);

		std::string get_lvalue() const;
		std::string get_rvalue() const;
		bool lvalue_is_nonprimitive() const;
		bool rvalue_is_nonprimitive() const;
		std::shared_ptr<bpp_entity> get_lvalue_object() const;
		std::shared_ptr<bpp_entity> get_rvalue_object() const;
		bool is_adding() const;
		bool rvalue_is_array() const;
};

/**
 * @class bpp_object_reference
 * 
 * @brief An object reference in Bash++
 * 
 * The 'bpp_' prefix signifies that this is used to parse a statement type which is unique to Bash++
 */
class bpp_object_reference : public bpp_string {
	private:
		bpp::reference_type reference_type;
		std::string array_index = "";
	public:	
		bpp_object_reference();

		void set_reference_type(bpp::reference_type reference_type);
		void set_array_index(const std::string& array_index);
		bpp::reference_type get_reference_type() const;
		std::string get_array_index() const;
};

/**
 * @class bpp_object_address
 * 
 * @brief A statement which takes the address of an object in Bash++
 * 
 * The 'bpp_' prefix signifies that this is used to parse a statement type which is unique to Bash++
 */
class bpp_object_address : public bpp_string {
	public:
		bpp_object_address() = default;
};

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_H_
