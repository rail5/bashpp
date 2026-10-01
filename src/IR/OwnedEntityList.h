/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <string_view>
#include <cstdint>
#include <span>

#include <IR/entities/Entity.h>
#include <IR/entities/components/Named.h>

namespace bpp::IR {

template <class T>
concept NamedEntity = std::is_base_of_v<Entity, T> && std::is_base_of_v<Components::Named, T>;

/**
 * @brief A list of entities that are owned by a parent entity. This class manages the lifetime of the entities it contains, and provides lookup by name.
 * 
 * @tparam T The type of entity to be stored in the list. Must be derived from both Entity and NamedEntity.
 */
template <NamedEntity T>
class OwnedEntityList {
	private:
		std::vector<std::unique_ptr<T>> entities;
		std::unordered_map<std::string_view, std::size_t> name_to_index;
	public:
		bool add(std::unique_ptr<T> entity) {
			const std::string_view name = entity->viewName();
			if (name_to_index.contains(name)) return false; // Entity with this name already exists
			entities.push_back(std::move(entity));
			name_to_index[name] = entities.size() - 1;
			return true;
		}

		T* find(std::string_view name, std::size_t max_visible_index = SIZE_MAX) const {
			auto it = name_to_index.find(name);
			if (it == name_to_index.end()) return nullptr; // No entity with this name
			std::size_t index = it->second;
			if (index > max_visible_index) return nullptr; // Entity exists but is out of bounds
			return entities[index].get();
		}

		std::size_t size() const {
			return entities.size();
		}

		std::span<const std::unique_ptr<T>> view_entities() const {
			return {entities.data(), entities.size()};
		}

		std::vector<std::unique_ptr<T>> release_entities() {
			name_to_index.clear();
			return std::move(entities);
		}

		OwnedEntityList() = default;
		~OwnedEntityList() = default;
		OwnedEntityList(const OwnedEntityList& other) = delete;
		OwnedEntityList& operator=(const OwnedEntityList& other) = delete;
		OwnedEntityList(OwnedEntityList&& other) noexcept = default;
		OwnedEntityList& operator=(OwnedEntityList&& other) noexcept = default;
};

} // namespace bpp::IR
