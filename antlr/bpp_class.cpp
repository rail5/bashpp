#include <ostream>
#include <string>
#include <vector>
#include <map>

enum BPP_SCOPE_ENUM {
	SCOPE_PUBLIC,
	SCOPE_PRIVATE,
	SCOPE_UNDEFINED
};

class BPP_SCOPE {
	private:
		BPP_SCOPE_ENUM scope;
	public:
		BPP_SCOPE() {
			scope = SCOPE_PRIVATE;
		}

		explicit BPP_SCOPE(BPP_SCOPE_ENUM scope) {
			this->scope = scope;
		}

		explicit BPP_SCOPE(std::string scope) {
			if (scope == "@public") {
				this->scope = SCOPE_PUBLIC;
			} else {
				this->scope = SCOPE_PRIVATE;
			}
		}

		BPP_SCOPE_ENUM getScope() const {
			return scope;
		}

		std::string toString() const {
			if (scope == SCOPE_PUBLIC) {
				return "@public";
			} else {
				return "@private";
			}
		}

		friend std::ostream& operator<<(std::ostream& os, const BPP_SCOPE& scope) {
			os << scope.toString();
			return os;
		}

		bool operator==(BPP_SCOPE_ENUM scope) {
			return this->scope == scope;
		}

		bool operator!=(BPP_SCOPE_ENUM scope) {
			return this->scope != scope;
		}

		bool operator==(BPP_SCOPE scope) {
			return this->scope == scope.getScope();
		}

		bool operator!=(BPP_SCOPE scope) {
			return this->scope != scope.getScope();
		}

		bool operator==(std::string scope) {
			return this->toString() == scope;
		}

		bool operator!=(std::string scope) {
			return this->toString() != scope;
		}

		BPP_SCOPE operator=(BPP_SCOPE_ENUM scope) {
			this->scope = scope;
			return *this;
		}

		BPP_SCOPE operator=(BPP_SCOPE scope) {
			this->scope = scope.getScope();
			return *this;
		}

		BPP_SCOPE operator=(const char* scope) {
			if (scope == "@public") {
				this->scope = SCOPE_PUBLIC;
			} else {
				this->scope = SCOPE_PRIVATE;
			}
			return *this;
		}

		BPP_SCOPE operator=(const std::string& scope) {
			if (scope == "@public") {
				this->scope = SCOPE_PUBLIC;
			} else {
				this->scope = SCOPE_PRIVATE;
			}
			return *this;
		}
};

struct bpp_class_variable {
	std::string variable_name;
	std::string variable_type;
	BPP_SCOPE variable_scope;
};

struct bpp_class_method {
	std::string method_name;
	std::string method_body;
	std::vector<bpp_class_variable> method_parameters;
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