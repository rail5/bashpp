/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/Entity.h>
#include <IR/entities/components/Named.h>
#include <IR/entities/components/Addressable.h>

#include <optional>
#include <variant>

namespace bpp::IR {

using OptionallyOwnedCodeEntity = std::variant<std::unique_ptr<CodeEntity>, const CodeEntity*>;

/**
 * @brief An object in Bash++.
 *
 * This includes both non-primitives and pointers.
 * Whether the object is a pointer, as well as its type, must be given in the constructor.
 */
class Object : public Entity, public Components::Named, public Components::Addressable {
	private:
		bool m_is_pointer = false;

		const Class* type = nullptr;

		// Initialization information:
		/// If a pointer or primitive, the initial value (if any)
		std::optional<OptionallyOwnedCodeEntity> initial_value;

		/// If not a pointer, the object from which this is copied (if any)
		const Object* copy_from = nullptr;
	public:
		Object() = default;
		Object(const Object& other);
		Object& operator=(const Object& other) = delete;
		Object(Object&& other) = default;
		Object& operator=(Object&& other) = default;
		~Object() override = default;

		std::string getAddress() const override;

		bool isPointer() const { return m_is_pointer; }
		void setIsPointer(bool is_pointer) { m_is_pointer = is_pointer; }

		const Class* getType() const { return type; }
		void setType(const Class* type) { this->type = type; }

		bool isPrimitive() const { return type == nullptr || isPointer(); }

		void setInitialValue(std::unique_ptr<CodeEntity> value);
		std::optional<const CodeEntity*> getInitialValue() const;
		bool hasInitialValue() const { return initial_value.has_value(); }

		/**
		 * @brief Release ownership of the initial value, if any, and return it.
		 * If no initial value is set, returns nullptr.
		 * @return std::unique_ptr<CodeEntity> The initial value, or nullptr if none is set.
		 */
		std::unique_ptr<CodeEntity> releaseInitialValue();

		void setCopyFrom(const Object* other) { copy_from = other; }
		const Object* getCopyFrom() const { return copy_from; }

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
