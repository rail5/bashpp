// Stub

#pragma once

#include <cstdint>
#include <string>

namespace bpp {
	enum class diagnostic_type : uint8_t {
		DIAGNOSTIC_ERROR,
		DIAGNOSTIC_WARNING
	};
}

namespace bpp::IR {

class Entity {
	public:
		virtual ~Entity() = default;
};

class Program : public Entity {
	public:
		virtual ~Program() = default;
		void add_diagnostic(std::string, bpp::diagnostic_type, std::string, uint32_t, uint32_t, uint32_t, uint32_t) {}
};

class Class : public Entity {
	public:
		virtual ~Class() = default;
};

} // namespace bpp::IR
