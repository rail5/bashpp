/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once
#include "interval_tree.hpp"
#include "tree_hooks.hpp"
#include <cstdint>

// Forward decl.
namespace bpp {
class bpp_entity;
}

using IntervalType = lib_interval_tree::interval<uint64_t, lib_interval_tree::closed>;

struct EntityNode : lib_interval_tree::node<uint64_t, IntervalType, EntityNode> {
	using base_type = lib_interval_tree::node<uint64_t, IntervalType, EntityNode>;
	using interval_type = typename base_type::interval_type;

	std::shared_ptr<bpp::bpp_entity> payload;

	EntityNode(EntityNode* parent, interval_type interval)
		: base_type(parent, std::move(interval)) {}
};

struct EntityHooks : lib_interval_tree::hooks::regular {
	using node_type = EntityNode;

	template <typename IteratorT>
	static inline bool on_find(IteratorT) noexcept { return true; }

	template <typename IteratorT>
	static inline bool on_overlap_find_all(
		std::conditional<true, 
			lib_interval_tree::interval_tree<lib_interval_tree::interval<uint64_t, 
					lib_interval_tree::closed>, 
					EntityHooks>, 
				const lib_interval_tree::interval_tree<lib_interval_tree::interval<uint64_t, 
					lib_interval_tree::closed>, 
					EntityHooks> 
			>::type&, 
		lib_interval_tree::interval_tree<lib_interval_tree::interval<uint64_t, 
				lib_interval_tree::closed>, 
				EntityHooks>::node_type*&, 
			const lib_interval_tree::interval_tree<lib_interval_tree::interval<uint64_t, 
				lib_interval_tree::closed>, 
				EntityHooks>::interval_type&
		) noexcept { return true; }
};

