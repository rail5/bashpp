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

class_body_statement: member_declaration
	| method_definition
	| constructor_definition
	| destructor_definition
	| DELIM
	| WS;

general_statement: include_statement
	| object_instantiation
	| pointer_declaration
	| object_assignment
	| object_reference
	| object_reference_as_lvalue
	| self_reference
	| self_reference_as_lvalue
	| delete_statement
	| supershell
	| subshell
	| deprecated_subshell
	| array_value
	| array_index
	| bash_arithmetic
	| string
	| singlequote_string
	| comment
	| object_address
	| pointer_dereference
	| nullptr_ref
	| bash_while_loop
	| bash_if_statement
	| bash_case_statement
	| heredoc
	| other_statement
	| DELIM
	| WS;

// Include statement
include_statement: AT (KEYWORD_INCLUDE | KEYWORD_INCLUDE_ONCE) WS* string;

// Class definition
class_definition: AT KEYWORD_CLASS WS* IDENTIFIER WS* (COLON WS* IDENTIFIER WS*)? LBRACE_ROOTLEVEL (class_body_statement | general_statement | extra_statement)* RBRACE_ROOTLEVEL;

// Member declarations
member_declaration: AT (KEYWORD_PUBLIC | KEYWORD_PRIVATE | KEYWORD_PROTECTED) WS* IDENTIFIER value_assignment?
	| AT (KEYWORD_PUBLIC | KEYWORD_PRIVATE | KEYWORD_PROTECTED) WS* (object_instantiation | pointer_declaration);

// Method definitions
method_definition: (AT KEYWORD_VIRTUAL WS*)? AT (KEYWORD_PUBLIC | KEYWORD_PRIVATE | KEYWORD_PROTECTED) WS* AT KEYWORD_METHOD WS* IDENTIFIER WS* parameter* WS* LBRACE (general_statement | class_body_statement)* RBRACE;

// Constructor definitions
constructor_definition: AT KEYWORD_CONSTRUCTOR WS* LBRACE general_statement* RBRACE;

// Destructor definitions
destructor_definition: AT KEYWORD_DESTRUCTOR WS* LBRACE general_statement* RBRACE;

// Value assignment
value_assignment: PLUS? ASSIGN ((raw_rvalue
	| string
	| singlequote_string
	| subshell
	| deprecated_subshell
	| bash_arithmetic
	| object_reference
	| self_reference
	| nullptr_ref
	| pointer_dereference
	| object_address
	| supershell)+
	| new_statement
	| array_value)?;

// Object instantiation
object_instantiation: AT IDENTIFIER_LVALUE WS* IDENTIFIER (value_assignment)?
	| AT IDENTIFIER WS* IDENTIFIER (value_assignment)?;

// Pointer declaration
pointer_declaration: AT IDENTIFIER_LVALUE ASTERISK WS* IDENTIFIER (value_assignment)?
	| AT IDENTIFIER ASTERISK WS* IDENTIFIER (value_assignment)?;

// Object assignment
object_assignment: (object_reference_as_lvalue | self_reference_as_lvalue) value_assignment;

// Pointer dereference
pointer_dereference: ASTERISK (object_reference | object_reference_as_lvalue | self_reference | self_reference_as_lvalue);

// Object address
object_address: AMPERSAND (object_reference | self_reference);

// Object reference
object_reference: AT IDENTIFIER (DOT IDENTIFIER)*
	| AT (LBRACE | LBRACE_ROOTLEVEL) POUNDKEY? IDENTIFIER (DOT IDENTIFIER)* array_index? (RBRACE | RBRACE_ROOTLEVEL);

object_reference_as_lvalue: AT IDENTIFIER_LVALUE (DOT IDENTIFIER)*
	| AT (LBRACE | LBRACE_ROOTLEVEL) POUNDKEY? IDENTIFIER_LVALUE (DOT IDENTIFIER)* array_index? (RBRACE | RBRACE_ROOTLEVEL);

// Self-reference from within a class
self_reference: AT KEYWORD_THIS (DOT IDENTIFIER)*
	| AT (LBRACE | LBRACE_ROOTLEVEL) POUNDKEY? KEYWORD_THIS (DOT IDENTIFIER)* array_index? (RBRACE | RBRACE_ROOTLEVEL);

self_reference_as_lvalue: AT KEYWORD_THIS_LVALUE (DOT IDENTIFIER)*
	| AT (LBRACE | LBRACE_ROOTLEVEL) POUNDKEY? KEYWORD_THIS_LVALUE (DOT IDENTIFIER)* array_index? (RBRACE | RBRACE_ROOTLEVEL);

// Delete statement
delete_statement: AT KEYWORD_DELETE WS* (object_reference | self_reference);

// Supershells
supershell: SUPERSHELL_START statement* SUPERSHELL_END;

// Subshells
subshell: SUBSHELL_START statement* SUBSHELL_END;

deprecated_subshell: DEPRECATED_SUBSHELL_START statement* DEPRECATED_SUBSHELL_END;

// Bash arithmetic
bash_arithmetic: BASH_ARITH_START statement* BASH_ARITH_END RPAREN;

// Strings
string: QUOTE (statement | terminal_token)* QUOTE_END;

singlequote_string: SINGLEQUOTE (statement | terminal_token)* SINGLEQUOTE_END;

// Heredocs
heredoc: heredoc_header (statement | terminal_token)* HEREDOC_END;

heredoc_header: HEREDOC_START statement* HEREDOC_CONTENT_START;

// Comments (skipped)
comment: COMMENT (statement | terminal_token)* (NEWLINE | EOF);

parameter: (IDENTIFIER | AT IDENTIFIER ASTERISK WS* IDENTIFIER) WS*;

nullptr_ref: AT KEYWORD_NULLPTR;

new_statement: AT KEYWORD_NEW WS* AT? IDENTIFIER;

array_value: ARRAY_ASSIGN_START statement* ARRAY_ASSIGN_END;

array_index: LBRACKET statement* RBRACKET;

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

// Other statement
other_statement: ~(RBRACE | RBRACE_ROOTLEVEL
	| RBRACKET | SUPERSHELL_END
	| QUOTE_END | SINGLEQUOTE_END
	| NEWLINE | SUBSHELL_END
	| DEPRECATED_SUBSHELL_END | BASH_ARITH_END
	| ARRAY_ASSIGN_END | BASH_KEYWORD_DONE
	| BASH_KEYWORD_DO
	| BASH_KEYWORD_IF | BASH_KEYWORD_ELIF
	| BASH_KEYWORD_THEN | BASH_KEYWORD_ELSE
	| BASH_KEYWORD_FI | BASH_CASE_PATTERN_DELIM
	| HEREDOC_END)+?;

// This rule will *only* ever be matched as part a value_assignment
raw_rvalue: IDENTIFIER | NUMBER | BASH_VAR;

extra_statement: RBRACE;

terminal_token: RBRACE_ROOTLEVEL | RBRACKET | BASH_KEYWORD_IF | BASH_KEYWORD_ELIF | BASH_KEYWORD_THEN | BASH_KEYWORD_ELSE | BASH_KEYWORD_FI | BASH_CASE_PATTERN_DELIM | BASH_KEYWORD_DONE | BASH_KEYWORD_DO;
