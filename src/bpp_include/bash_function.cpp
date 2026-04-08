/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include "bpp.h"
#include "bpp_codegen.h"

namespace bpp {

void bash_function::destruct_local_objects(std::shared_ptr<bpp_program> program) {
	for (auto& o : local_objects) {
		if (o.second->is_pointer()) continue;

		code_segment delete_code = generate_delete_code(o.second, o.second->get_address(), program);

		*code << delete_code.full_code() << "\n" << std::flush;
	}
}

} // namespace bpp
