#include <iostream>
#include <string>
#include <vector>
#include <limits>

#include "bpp_classes.cpp"

int main(int argc, char* argv[]) {
	bool create = false;
	std::vector<bpp_class> classes;
	do {
		bpp_class newClass;
		std::cout << "Enter the name of the class: ";
		std::cin >> newClass.name;

		std::string data_member_name = "";
		do {
			std::cout << "Enter name of data member (type 'done' when finished): ";
			std::cin >> data_member_name;
			if (data_member_name == "done") {
				break;
			}
			std::string data_member_default_value = "";
			std::cout << "Enter default value for data member: ";
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::getline(std::cin, data_member_default_value);

			newClass.data_members[data_member_name] = data_member_default_value;
		} while (data_member_name != "done");
		
		std::string method_name = "";
		do {
			std::cout << "Enter name of method (type 'done' when finished): ";
			std::cin >> method_name;
			if (method_name == "done") {
				break;
			}

			bpp_method newMethod;
			newMethod.name = method_name;

			std::string param_name = "";
			std::string param_type = "";
			do {
				std::cout << "Enter name of parameter for method '" << method_name << "' (type 'done' when finished): ";
				std::cin >> param_name;
				if (param_name == "done") {
					break;
				}
				std::cout << "Enter type of parameter: ";
				std::cin >> param_type;

				bpp_param newParam;
				newParam.name = param_name;
				newParam.type = (param_type == "") ? "primitive" : param_type;

				newMethod.parameters.push_back(newParam);
			} while (param_name != "done");

			std::string method_body = "";
			std::cout << "Enter body of method '" << method_name << "' (type 'EOF' to signal end of body): " << std::endl;
			std::string line;
			while (std::getline(std::cin, line)) {
				if (line == "EOF") {
					break;
				}
				method_body += line + "\n";
			}
			newMethod.body = method_body;

			newClass.methods.push_back(newMethod);
		} while (method_name != "done");

		classes.push_back(newClass);

		std::cout << "Would you like to create another class? (y/n): ";
		std::string response;
		std::cin >> response;

		switch (response[0]) {
			case 'Y':
			case 'y':
				create = true;
				break;
			default:
				create = false;
				break;
		}

	} while (create == true);

	for (const bpp_class& cls : classes) {
		std::cerr << cls.write_class() << std::endl;
	}

	return 0;
}