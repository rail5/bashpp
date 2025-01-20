/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BPP_H_
#define SRC_BPP_INCLUDE_BPP_H_

#include <string>
#include <vector>
#include <map>
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

class bpp_entity;
class bpp_code_entity;
class bpp_program;
class bpp_class;
class bpp_method;
class bpp_method_parameter;
class bpp_datamember;
class bpp_constructor;
class bpp_destructor;
class bpp_object;

class bpp_entity {
	protected:
		std::map<std::string, std::shared_ptr<bpp_class>> classes;
		std::map<std::string, std::shared_ptr<bpp_object>> objects;
		std::map<std::string, std::shared_ptr<bpp_object>> local_objects;
		std::shared_ptr<bpp_class> type = nullptr;
		std::weak_ptr<bpp_class> containing_class;
		std::vector<std::shared_ptr<bpp_class>> parents;
	public:
		virtual ~bpp_entity() = default;
		virtual bool add_class(std::shared_ptr<bpp_class> class_);
		virtual bool add_object(std::shared_ptr<bpp_object> object);

		virtual std::shared_ptr<bpp_class> get_class() const;
		virtual std::string get_address() const;
		virtual std::string get_name() const;
		virtual std::weak_ptr<bpp::bpp_class> get_containing_class() const;
		virtual bool set_containing_class(std::weak_ptr<bpp::bpp_class> containing_class);

		virtual void inherit(std::shared_ptr<bpp_entity> parent);
		virtual void inherit(std::shared_ptr<bpp_class> parent);

		bool is_child_of(std::shared_ptr<bpp_entity> parent);

		virtual std::vector<std::shared_ptr<bpp_class>> get_classes() const;
		virtual std::vector<std::shared_ptr<bpp_object>> get_objects() const;
		virtual std::shared_ptr<bpp_class> get_class(std::string name);
		virtual std::shared_ptr<bpp_object> get_object(std::string name);
};

const std::shared_ptr<bpp_entity> inaccessible_entity = std::make_shared<bpp_entity>();
const std::shared_ptr<bpp_datamember> inaccessible_datamember = std::make_shared<bpp_datamember>();
const std::shared_ptr<bpp_method> inaccessible_method = std::make_shared<bpp_method>();

const char bpp_nullptr[] = "0";

class bpp_code_entity : public bpp_entity {
	protected:
		std::string code = "";
		std::string nextline_buffer = "";
		std::string postline_buffer = "";
	public:
		bpp_code_entity();
		virtual ~bpp_code_entity() = default;

		virtual void add_code(std::string code);
		virtual void add_code_to_previous_line(std::string code);
		virtual void add_code_to_next_line(std::string code);

		virtual void flush_nextline_buffer();
		virtual void flush_postline_buffer();
		virtual void flush_code_buffers();

		virtual std::string get_code() const;
		virtual std::string get_pre_code() const;
		virtual std::string get_post_code() const;
};

class bpp_string : public bpp_code_entity {
	public:
		bpp_string();

		void add_code(std::string code) override;
		void add_code_to_previous_line(std::string code) override;
		void add_code_to_next_line(std::string code) override;

		std::string get_code() const override;
		std::string get_pre_code() const override;
		std::string get_post_code() const override;
};

class bpp_value_assignment : public bpp_string {
	private:
		bool nonprimitive_assignment = false;
		std::shared_ptr<bpp_entity> nonprimitive_object;
		bool lvalue_nonprimitive = false;
	public:
		bpp_value_assignment();

		void set_nonprimitive_assignment(bool is_nonprimitive);
		void set_nonprimitive_object(std::shared_ptr<bpp_entity> object);
		void set_lvalue_nonprimitive(bool is_nonprimitive);

		bool is_nonprimitive_assignment() const;
		std::shared_ptr<bpp_entity> get_nonprimitive_object() const;
		bool lvalue_is_nonprimitive() const;
};

class bpp_object_assignment : public bpp_string {
	private:
		std::string lvalue = "";
		std::string rvalue = "";

		bool lvalue_nonprimitive = false;
		bool rvalue_nonprimitive = false;

		std::shared_ptr<bpp_entity> lvalue_object;
		std::shared_ptr<bpp_entity> rvalue_object;
	public:
		bpp_object_assignment();

		void set_lvalue(const std::string& lvalue);
		void set_rvalue(const std::string& rvalue);
		void set_lvalue_nonprimitive(bool is_nonprimitive);
		void set_rvalue_nonprimitive(bool is_nonprimitive);
		void set_lvalue_object(std::shared_ptr<bpp_entity> object);
		void set_rvalue_object(std::shared_ptr<bpp_entity> object);

		std::string get_lvalue() const;
		std::string get_rvalue() const;
		bool lvalue_is_nonprimitive() const;
		bool rvalue_is_nonprimitive() const;
		std::shared_ptr<bpp_entity> get_lvalue_object() const;
		std::shared_ptr<bpp_entity> get_rvalue_object() const;
};

class bpp_object_reference : public bpp_string {
	private:
		bpp::reference_type reference_type;
	public:	
		bpp_object_reference();

		void set_reference_type(bpp::reference_type reference_type);
		bpp::reference_type get_reference_type() const;
};

class bpp_object_address : public bpp_string {
	public:
		bpp_object_address() = default;
};

class bpp_method : public bpp_code_entity {
	private:
		std::string name;
		std::vector<std::shared_ptr<bpp_method_parameter>> parameters;
		bpp_scope scope = SCOPE_PRIVATE;
		bool m_is_virtual = false;
	public:
		bpp_method();
		explicit bpp_method(std::string name);

		bool add_object(std::shared_ptr<bpp_object> object) override;

		virtual bool add_parameter(std::shared_ptr<bpp_method_parameter> parameter);
		void set_name(std::string name);
		void set_scope(bpp_scope scope);
		void set_virtual(bool is_virtual);

		std::string get_name() const;
		std::vector<std::shared_ptr<bpp_method_parameter>> get_parameters() const;
		bpp_scope get_scope() const;
		bool is_virtual() const;

		void destruct_local_objects();

		void destroy();
};

class bpp_constructor : public bpp_method {
	public:
		bpp_constructor();
		explicit bpp_constructor(std::string name);

		bool add_parameter(std::shared_ptr<bpp_method_parameter> parameter) override;
};

class bpp_destructor : public bpp_method {
	public:
		bpp_destructor();
		explicit bpp_destructor(std::string name);

		bool add_parameter(std::shared_ptr<bpp_method_parameter> parameter) override;
};

class bpp_method_parameter : public bpp_entity {
	private:
		std::shared_ptr<bpp_class> type;
		std::string name;
	public:
		explicit bpp_method_parameter(std::string name);

		void set_type(std::shared_ptr<bpp_class>);

		std::string get_name() const;
		std::shared_ptr<bpp_class> get_type() const;
};

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

		void remove_default_toPrimitive() {
			if (!has_custom_toPrimitive) {
				// Remove the toPrimitive method from the methods vector
				for (auto it = methods.begin(); it != methods.end(); it++) {
					if ((*it)->get_name() == "toPrimitive") {
						methods.erase(it);
						break;
					}
				}
			}
		}

		void add_default_toPrimitive() {
			if (!has_custom_toPrimitive) {
				std::shared_ptr<bpp_method> toPrimitive = std::make_shared<bpp_method>();
				toPrimitive->set_name("toPrimitive");
				std::string default_toPrimitive_body = "	echo " + name + " Instance\n";
				toPrimitive->add_code(default_toPrimitive_body);
				toPrimitive->set_scope(bpp_scope::SCOPE_PUBLIC);
				remove_default_toPrimitive();
				methods.push_back(toPrimitive);
			}
		}
	public:
		bpp_class();
		explicit bpp_class(std::string name);
		bpp_class(const bpp_class& parent, std::string name);

		std::weak_ptr<bpp_class> get_containing_class() const override;
		bool set_containing_class(std::weak_ptr<bpp::bpp_class> containing_class) override;

		std::shared_ptr<bpp_class> get_class() const override;

		void set_name(std::string name);
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

		std::shared_ptr<bpp_method> get_method(std::string name, std::shared_ptr<bpp_entity> context);
		std::shared_ptr<bpp_datamember> get_datamember(std::string name, std::shared_ptr<bpp_entity> context);

		void inherit(std::shared_ptr<bpp_class> parent) override;

		void destroy();
};

class bpp_object : public bpp_entity {
	protected:
		std::string name = "";
		std::string address = "";
		std::string assignment_value = "";
		std::string pre_access_code = "";
		std::string post_access_code = "";
		std::shared_ptr<bpp_class> type;
		bool m_is_pointer = false;
	public:
		bpp_object();
		explicit bpp_object(std::string name);
		bpp_object(std::string name, bool is_pointer);

		void set_class(std::shared_ptr<bpp_class> object_class);
		void set_pointer(bool is_pointer);
		void set_name(std::string name);
		void set_address(std::string address);
		void set_assignment_value(std::string assignment_value);
		void set_pre_access_code(std::string pre_access_code);
		void set_post_access_code(std::string post_access_code);
		void set_nullptr();

		std::string get_name() const;
		virtual std::string get_address() const;
		std::string get_assignment_value() const;
		std::shared_ptr<bpp_class> get_class() const;
		std::string get_pre_access_code() const;
		std::string get_post_access_code() const;
		bool is_nullptr() const;

		bool is_pointer() const;
};

class bpp_datamember : public bpp_object {
	private:
		std::string default_value = "";
		bpp_scope scope = SCOPE_PRIVATE;
	public:
		bpp_datamember();

		void set_default_value(std::string default_value);
		
		void set_scope(bpp_scope scope);

		std::string get_address() const override;
		std::string get_default_value() const;
		bpp_scope get_scope() const;

		void destroy();
};

class bpp_program : public bpp_code_entity {
	private:
		std::shared_ptr<bpp_class> primitive_class;
	public:
		bpp_program();

		bool set_containing_class(std::weak_ptr<bpp_class> containing_class) override;

		bool add_class(std::shared_ptr<bpp_class> class_) override;
		bool add_object(std::shared_ptr<bpp_object> object) override;

		void prepend_code(std::string code);

		std::shared_ptr<bpp_class> get_primitive_class() const;
};

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_H_
