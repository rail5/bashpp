/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "SourceIndex.h"

#include <IR/entities/Program.h>

namespace bpp::IR {

views::path_view SourceIndex::fetchOrAdd(const std::filesystem::path& path) {
	auto it = std::find(paths.begin(), paths.end(), path);
	if (it == paths.end()) it = paths.insert(paths.end(), path);
	return {*it};
}

void SourceIndex::addDiagnostic(const std::filesystem::path& source_file, const bpp::ErrorHandling::Diagnostic& diagnostic) {
	auto path = fetchOrAdd(source_file);
	files[path].diagnostics.push_back(diagnostic);
}

void SourceIndex::addReference(const Entity* entity, const SourceLocationRange& range) {
	auto path = fetchOrAdd(range.source_file);
	files[path].entity_references.insert(range.start, range.end, entity);
	entities[entity].references.emplace_back(path, range.start, range.end);
}

void SourceIndex::setDefinitionPosition(const Entity* entity, const SourceLocationRange& range) {
	auto path = fetchOrAdd(range.source_file);
	files[path].entity_references.insert(range.start, range.end, entity);
	entities[entity].definition_position = {path, range.start, range.end};
}

void SourceIndex::markContextEntity(const SourceLocationRange& range, const Entity* entity) {
	auto path = fetchOrAdd(range.source_file);
	files[path].context_entities.insert(range.start, range.end, entity);
}

std::span<const bpp::ErrorHandling::Diagnostic> SourceIndex::getDiagnostics(views::path_view source_file) const {
	auto it = files.find(source_file);
	if (it == files.end()) return {};
	return it->second.diagnostics;
}

std::span<const bpp::ErrorHandling::Diagnostic> SourceIndex::getDiagnostics(const std::filesystem::path& source_file) const {
	return getDiagnostics(views::path_view(source_file));
}

std::span<const views::SourceLocationRangeView> SourceIndex::getReferences(const Entity* entity) const {
	auto it = entities.find(entity);
	if (it == entities.end()) return {};
	return it->second.references;
}

std::optional<views::SourceLocationRangeView> SourceIndex::getDefinitionPosition(const Entity* entity) const {
	auto it = entities.find(entity);
	if (it == entities.end() || !it->second.definition_position.has_value()) return std::nullopt;
	return it->second.definition_position;
}

const Entity* SourceIndex::getReferencedEntity(views::SourceLocationView location) const {
	auto it = files.find(location.source_file);
	if (it == files.end()) return nullptr;
	return it->second.entity_references.find(location.position.line, location.position.column);
}

const Entity* SourceIndex::getReferencedEntity(const SourceLocation& location) const {
	return getReferencedEntity(views::SourceLocationView(location));
}

const Entity* SourceIndex::getContextEntity(views::SourceLocationView location) const {
	const Entity* result = program; // If no context entity is found, return the root program as the context entity
	auto it = files.find(location.source_file);
	if (it == files.end()) return result;
	const auto* ctx = it->second.context_entities.find(location.position.line, location.position.column);
	if (ctx) result = ctx;
	return result;
}

const Entity* SourceIndex::getContextEntity(const SourceLocation& location) const {
	return getContextEntity(views::SourceLocationView(location));
}

void SourceIndex::updateEntityAddress(const Entity* old_address, const Entity* new_address) {
	auto it = entities.find(old_address);
	if (it != entities.end()) {
		auto entity_data = std::move(it->second);
		entities.erase(it);
		entities[new_address].mergeFrom(std::move(entity_data));
	}

	for (auto& [path, file_data] : files) {
		file_data.context_entities.updateEntityAddress(old_address, new_address);
		file_data.entity_references.updateEntityAddress(old_address, new_address);
	}
}

} // namespace bpp::IR
