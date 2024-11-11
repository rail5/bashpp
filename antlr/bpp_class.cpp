#include <string>
#include <vector>
#include <map>

enum BPP_SCOPE {
	SCOPE_PUBLIC,
	SCOPE_PRIVATE
};

struct bpp_class_variable {
	std::string variable_name;
	std::string variable_type;
	BPP_SCOPE variable_scope;
};

struct bpp_class_method {
	std::string method_name;
	std::string method_body;
	BPP_SCOPE method_scope;
};

struct bpp_class_constructor {
	std::string constructor_body;
};

struct bpp_class {
	std::string class_name;
	std::map<std::string, bpp_class_variable> variables;
	std::map<std::string, bpp_class_method> methods;
	std::vector<bpp_class_constructor> constructors;
};