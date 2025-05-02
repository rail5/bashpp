parser grammar BashppParser;

options {tokenVocab=BashppLexer;}

// TODO(@rail5): The parser is slow, and relies too much on ANTLR's lookahead and backtracking.
// We should be able to parse using SLL if we're careful about restructuring the grammar to remove ambiguity.

// Program
program: (statement | terminal_token)*;

// Statements
statement: (class_definition
	| class_body_statement
	| general_statement
	| extra_statement) DELIM?;

class_body_statement: class_member_or_method
	| constructor_definition
	| destructor_definition
	| DELIM
	| WS;

general_statement: include_statement
	| object_instantiation
	| pointer_declaration
	| object_assignment
	| ref_general
	| delete_statement
	| supershell
	| subshell
	| deprecated_subshell
	| array_value
	| array_index
	| bash_arithmetic
	| string
	| singlequote_string
	| object_address
	| pointer_dereference
	| nullptr_ref
	| dynamic_cast_statement
	| new_statement
	| bash_while_loop
	| bash_if_statement
	| bash_case_statement
	| bash_for_loop
	| heredoc
	| other_statement
	| DELIM
	| WS;

// Include statement
include_statement: (KEYWORD_INCLUDE | KEYWORD_INCLUDE_ONCE) JUNK*
	(INCLUDE_STATIC | INCLUDE_DYNAMIC)? JUNK*
	(LOCAL_INCLUDE_PATH | SYSTEM_INCLUDE_PATH) JUNK*
	(INCLUDE_AS LOCAL_INCLUDE_PATH)? JUNK*;

// Class definition
class_definition: KEYWORD_CLASS WS* IDENTIFIER WS* (COLON WS* IDENTIFIER WS*)? LBRACE_ROOTLEVEL (class_body_statement | general_statement | extra_statement)* RBRACE_ROOTLEVEL;

class_member_or_method: (KEYWORD_VIRTUAL WS*)? (KEYWORD_PUBLIC | KEYWORD_PRIVATE | KEYWORD_PROTECTED) WS* (member_declaration | method_definition);

// Member declarations
member_declaration: IDENTIFIER value_assignment?
	| object_instantiation
	| pointer_declaration;

// Method definitions
method_definition: KEYWORD_METHOD WS* IDENTIFIER WS* parameter* WS* METHOD_START (class_body_statement | general_statement | extra_statement)* METHOD_END;

// Constructor definitions
constructor_definition: KEYWORD_CONSTRUCTOR WS* LBRACE general_statement* RBRACE;

// Destructor definitions
destructor_definition: KEYWORD_DESTRUCTOR WS* LBRACE general_statement* RBRACE;

// Value assignment
value_assignment: PLUS? ASSIGN valid_rvalue;

valid_rvalue: ((raw_rvalue
	| string
	| singlequote_string
	| subshell
	| deprecated_subshell
	| bash_arithmetic
	| ref_rvalue
	| nullptr_ref
	| pointer_dereference
	| object_address
	| supershell)+
	| new_statement
	| dynamic_cast_statement
	| array_value)?;

// Object instantiation
object_instantiation: AT IDENTIFIER_LVALUE WS* IDENTIFIER (value_assignment)?
	| AT IDENTIFIER WS* IDENTIFIER (value_assignment)?;

// Pointer declaration
pointer_declaration: AT IDENTIFIER_LVALUE ASTERISK WS* IDENTIFIER (value_assignment)?
	| AT IDENTIFIER ASTERISK WS* IDENTIFIER (value_assignment)?;

// Object assignment
object_assignment: ref_lvalue value_assignment;

// Pointer dereference
pointer_dereference: ASTERISK ref_general;

// Object address
object_address: AMPERSAND ref_rvalue;

// Casting
dynamic_cast_statement: KEYWORD_DYNAMIC_CAST WS* LESSTHAN WS* IDENTIFIER WS* ASTERISK? WS* GREATERTHAN WS* valid_rvalue;

// Object reference
ref_general: ref_lvalue | ref_rvalue;

ref_lvalue: AT (object_reference_as_lvalue | self_reference_as_lvalue)
	| AT (LBRACE | LBRACE_ROOTLEVEL) POUNDKEY? (object_reference_as_lvalue | self_reference_as_lvalue) (RBRACE | RBRACE_ROOTLEVEL);

ref_rvalue: AT (object_reference | self_reference)
	| AT (LBRACE | LBRACE_ROOTLEVEL) POUNDKEY? (object_reference | self_reference) (RBRACE | RBRACE_ROOTLEVEL);

object_reference: IDENTIFIER (DOT IDENTIFIER)* array_index?;

object_reference_as_lvalue: IDENTIFIER_LVALUE (DOT IDENTIFIER)* array_index?;

// Self-reference from within a class
self_reference: KEYWORD_THIS (DOT IDENTIFIER)* array_index?;

self_reference_as_lvalue: KEYWORD_THIS_LVALUE (DOT IDENTIFIER)* array_index?;

// Delete statement
delete_statement: KEYWORD_DELETE WS* ref_rvalue;

// Supershells
supershell: SUPERSHELL_START statement* SUPERSHELL_END;

// Subshells
subshell: SUBSHELL_START statement* SUBSHELL_END;

deprecated_subshell: DEPRECATED_SUBSHELL_START statement* DEPRECATED_SUBSHELL_END;

// Bash arithmetic
bash_arithmetic: BASH_ARITH_START statement* BASH_ARITH_END;

// Strings
string: QUOTE (statement | terminal_token)* QUOTE_END;

singlequote_string: SINGLEQUOTE (statement | terminal_token)* SINGLEQUOTE_END;

// Heredocs
heredoc: heredoc_header (statement | terminal_token)* HEREDOC_END;

heredoc_header: HEREDOC_START statement* HEREDOC_CONTENT_START;

parameter: (IDENTIFIER | AT IDENTIFIER ASTERISK WS* IDENTIFIER) WS*;

nullptr_ref: KEYWORD_NULLPTR;

new_statement: KEYWORD_NEW WS* AT? IDENTIFIER;

array_value: ARRAY_ASSIGN_START statement* ARRAY_ASSIGN_END;

array_index: ARRAY_INDEX_START statement* ARRAY_INDEX_END;

// Bash if statements
bash_if_statement: bash_if_root_branch bash_if_else_branch* BASH_KEYWORD_FI;

bash_if_root_branch: BASH_KEYWORD_IF bash_if_condition statement*;

bash_if_else_branch: (BASH_KEYWORD_ELIF bash_if_condition | BASH_KEYWORD_ELSE) statement*;

bash_if_condition: statement* DELIM WS* BASH_KEYWORD_THEN; // Kind of a hack
	// Meant to catch the condition of an if statement
	// As in 'if [[ "@this.member" == "value" ]]; then'
	// Or 'if command; then'
	// Without caring whether it started with 'if' or 'elif'

// Bash while loops
bash_while_loop: bash_while_condition statement* BASH_KEYWORD_DONE;

bash_while_condition: BASH_KEYWORD_WHILE statement* DELIM WS* BASH_KEYWORD_DO;

// Bash case statements
bash_case_statement: BASH_KEYWORD_CASE statement* BASH_KEYWORD_IN (WS | DELIM)* bash_case_pattern* (WS | DELIM)* BASH_KEYWORD_ESAC;

bash_case_pattern: bash_case_pattern_header statement* BASH_CASE_PATTERN_DELIM;

bash_case_pattern_header: statement* RPAREN;

// Bash for statements
bash_for_loop: bash_for_header statement* BASH_KEYWORD_DONE;

bash_for_header: BASH_KEYWORD_FOR (WS | DELIM)* (
	((IDENTIFIER | INVALID_IDENTIFIER) (WS | DELIM)* BASH_KEYWORD_IN statement*)
	| bash_arithmetic)
	DELIM WS* BASH_KEYWORD_DO;

// Other statement
other_statement: ~(RBRACE | RBRACE_ROOTLEVEL
	| ARRAY_INDEX_END | SUPERSHELL_END
	| QUOTE_END | SINGLEQUOTE_END
	| NEWLINE | SUBSHELL_END
	| DEPRECATED_SUBSHELL_END | BASH_ARITH_END
	| ARRAY_ASSIGN_END | BASH_KEYWORD_DONE
	| BASH_KEYWORD_DO | METHOD_END
	| BASH_KEYWORD_IF | BASH_KEYWORD_ELIF
	| BASH_KEYWORD_THEN | BASH_KEYWORD_ELSE
	| BASH_KEYWORD_FI | BASH_CASE_PATTERN_DELIM
	| HEREDOC_END)+?;

// This rule will *only* ever be matched as part a value_assignment
raw_rvalue: IDENTIFIER | NUMBER | BASH_VAR;

extra_statement: RBRACE;

terminal_token: RBRACE_ROOTLEVEL | ARRAY_INDEX_END | BASH_KEYWORD_IF | BASH_KEYWORD_ELIF | BASH_KEYWORD_THEN | BASH_KEYWORD_ELSE | BASH_KEYWORD_FI | BASH_CASE_PATTERN_DELIM | BASH_KEYWORD_DONE | BASH_KEYWORD_DONE;
