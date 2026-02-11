/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include "bpp.h"

namespace bpp {

void bpp_delete_statement::set_object_to_delete(std::shared_ptr<bpp_object> object) {
	this->object_to_delete = object;
}

std::shared_ptr<bpp_object> bpp_delete_statement::get_object_to_delete() const {
	return object_to_delete;
}

} // namespace bpp
