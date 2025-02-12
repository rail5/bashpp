/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_HANDLERS_H_
#define SRC_HANDLERS_H_

#include "listener/handlers/class_body_statement.cpp"
#include "listener/handlers/class_definition.cpp"
#include "listener/handlers/comment.cpp"
#include "listener/handlers/constructor_definition.cpp"
#include "listener/handlers/delete_statement.cpp"
#include "listener/handlers/deprecated_subshell.cpp"
#include "listener/handlers/bash_arithmetic.cpp"
#include "listener/handlers/destructor_definition.cpp"
#include "listener/handlers/general_statement.cpp"
#include "listener/handlers/include_statement.cpp"
#include "listener/handlers/member_declaration.cpp"
#include "listener/handlers/method_definition.cpp"
#include "listener/handlers/new_statement.cpp"
#include "listener/handlers/nullptr_ref.cpp"
#include "listener/handlers/object_address.cpp"
#include "listener/handlers/object_assignment.cpp"
#include "listener/handlers/object_instantiation.cpp"
#include "listener/handlers/object_reference_as_lvalue.cpp"
#include "listener/handlers/object_reference.cpp"
#include "listener/handlers/other_statement.cpp"
#include "listener/handlers/parameter.cpp"
#include "listener/handlers/pointer_declaration.cpp"
#include "listener/handlers/pointer_dereference.cpp"
#include "listener/handlers/program.cpp"
#include "listener/handlers/raw_rvalue.cpp"
#include "listener/handlers/self_reference_as_lvalue.cpp"
#include "listener/handlers/self_reference.cpp"
#include "listener/handlers/singlequote_string.cpp"
#include "listener/handlers/statement.cpp"
#include "listener/handlers/string.cpp"
#include "listener/handlers/subshell.cpp"
#include "listener/handlers/supershell.cpp"
#include "listener/handlers/heredoc.cpp"
#include "listener/handlers/value_assignment.cpp"
#include "listener/handlers/array_value.cpp"
#include "listener/handlers/array_index.cpp"
#include "listener/handlers/typecast.cpp"
#include "listener/handlers/bash_while_declaration.cpp"
#include "listener/handlers/bash_if_statement.cpp"
#include "listener/handlers/bash_case_statement.cpp"
#include "listener/handlers/extra_statement.cpp"

#endif // SRC_HANDLERS_H_
