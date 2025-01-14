/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_BPP_INCLUDE_BPP_H_
#define ANTLR_BPP_INCLUDE_BPP_H_

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "replace_all.cpp"

namespace bpp {

enum bpp_scope {
	SCOPE_PUBLIC,
	SCOPE_PROTECTED,
	SCOPE_PRIVATE
};

class bpp_program;
class bpp_class;
class bpp_method;
class bpp_method_parameter;
class bpp_datamember;
class bpp_constructor;
class bpp_destructor;
class bpp_object;

class bpp_datamember {
	private:
		std::string type = "primitive";
		std::string name;
		std::string default_value = "";
		bpp_scope scope = SCOPE_PRIVATE;
	public:
		explicit bpp_datamember(std::string name);

		void set_type(std::string type);
		void set_default_value(std::string default_value);
		void set_scope(bpp_scope scope);

		std::string get_name() const;
		std::string get_type() const;
		std::string get_default_value() const;
		bpp_scope get_scope() const;
};

class bpp_method {
	private:
		std::string name;
		std::vector<bpp_method_parameter> parameters;
		std::string method_body;
		bpp_scope scope = SCOPE_PRIVATE;
		bool m_is_virtual = false;
	public:
		explicit bpp_method(std::string name);

		virtual bool add_parameter(bpp_method_parameter parameter);
		void set_name(std::string name);
		void set_method_body(std::string method_body);
		void set_scope(bpp_scope scope);
		void set_virtual(bool is_virtual);

		std::string get_name() const;
		std::vector<bpp_method_parameter> get_parameters() const;
		std::string get_method_body() const;
		bpp_scope get_scope() const;
		bool is_virtual() const;

		virtual std::string get_signature() const;
};

class bpp_constructor : public bpp_method {
	public:
		bpp_constructor();
		explicit bpp_constructor(std::string name);

		bool add_parameter(bpp_method_parameter parameter) override;
};

class bpp_destructor : public bpp_method {
	public:
		bpp_destructor();
		explicit bpp_destructor(std::string name);

		bool add_parameter(bpp_method_parameter parameter) override;
};

class bpp_method_parameter {
	private:
		std::string type = "primitive";
		std::string name;
	public:
		explicit bpp_method_parameter(std::string name);

		void set_type(std::string type);

		std::string get_name() const;
		std::string get_type() const;
};

class bpp_class {
	private:
		std::string name;
		std::vector<bpp_method> methods;
		std::vector<bpp_datamember> datamembers;
		bpp_constructor constructor;
		bpp_destructor destructor;
		bool constructor_set = false;
		bool destructor_set = false;
	public:
		bpp_class();
		explicit bpp_class(std::string name);
		bpp_class(const bpp_class& parent, std::string name);

		void set_name(std::string name);
		bool add_method(bpp_method method);
		bool add_datamember(bpp_datamember datamember);
		bool set_constructor(bpp_constructor constructor);
		bool set_destructor(bpp_destructor destructor);

		std::string get_name() const;
		std::vector<bpp_method> get_methods() const;
		std::vector<bpp_datamember> get_datamembers() const;
		bpp_constructor get_constructor() const;
		bpp_destructor get_destructor() const;
		bool has_constructor() const;
		bool has_destructor() const;
};

class bpp_object {
	private:
		std::string name;
		std::string address;
		std::shared_ptr<bpp_class> object_class;
		bool m_is_pointer = false;
	public:
		bpp_object();
		explicit bpp_object(std::string name);
		bpp_object(std::string name, bool is_pointer);

		void set_class(std::shared_ptr<bpp_class> object_class);
		void set_pointer(bool is_pointer);
		void set_name(std::string name);
		void set_address(std::string address);

		std::string get_name() const;
		std::string get_address() const;
		std::shared_ptr<bpp_class> get_class() const;
		bool is_pointer() const;
};

class bpp_program {
	private:
		std::map<std::string, bpp_class> classes;
		std::map<std::string, bpp_object> objects;
		std::string code = "#!/usr/bin/env bash\n";
	public:
		bpp_program();

		bool add_class(bpp_class class_);
		bool add_object(bpp_object object);
		void add_code(std::string code);

		std::vector<bpp_class> get_classes() const;
		std::vector<bpp_object> get_objects() const;
		std::string get_code() const;
};

} // namespace bpp

#endif // ANTLR_BPP_INCLUDE_BPP_H_
