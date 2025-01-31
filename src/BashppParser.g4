parser grammar BashppParser;

options {tokenVocab=BashppLexer;}

// Program
program: statement*;

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
	| bash_arithmetic
	| string
	| singlequote_string
	| comment
	| object_address
	| pointer_dereference
	| typecast
	| nullptr_ref
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
	| typecast
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
pointer_dereference: ASTERISK (object_reference | self_reference);

// Object address
object_address: AMPERSAND (object_reference | self_reference);

// Object reference
object_reference: AT IDENTIFIER (DOT IDENTIFIER)*
	| AT (LBRACE | LBRACE_ROOTLEVEL) POUNDKEY? IDENTIFIER (DOT IDENTIFIER)* (LBRACKET (AT | BASH_VAR | NUMBER) RBRACKET)? (RBRACE | RBRACE_ROOTLEVEL);

object_reference_as_lvalue: AT IDENTIFIER_LVALUE (DOT IDENTIFIER)*
	| AT (LBRACE | LBRACE_ROOTLEVEL) POUNDKEY? IDENTIFIER_LVALUE (DOT IDENTIFIER)* (LBRACKET (AT | BASH_VAR | NUMBER) RBRACKET)? (RBRACE | RBRACE_ROOTLEVEL);

// Self-reference from within a class
self_reference: AT KEYWORD_THIS (DOT IDENTIFIER)*
	| AT (LBRACE | LBRACE_ROOTLEVEL) POUNDKEY? KEYWORD_THIS (DOT IDENTIFIER)* (LBRACKET (AT | BASH_VAR | NUMBER) RBRACKET)? (RBRACE | RBRACE_ROOTLEVEL);

self_reference_as_lvalue: AT KEYWORD_THIS_LVALUE (DOT IDENTIFIER)*
	| AT (LBRACE | LBRACE_ROOTLEVEL) POUNDKEY? KEYWORD_THIS_LVALUE (DOT IDENTIFIER)* (LBRACKET (AT | BASH_VAR | NUMBER) RBRACKET)? (RBRACE | RBRACE_ROOTLEVEL);

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
string: QUOTE statement* QUOTE_END;

singlequote_string: SINGLEQUOTE statement* SINGLEQUOTE_END;

// Comments (skipped)
comment: COMMENT statement* NEWLINE;

parameter: (IDENTIFIER | AT IDENTIFIER ASTERISK WS* IDENTIFIER) WS*;

nullptr_ref: AT KEYWORD_NULLPTR;

new_statement: AT KEYWORD_NEW WS* AT? IDENTIFIER;

typecast: (KEYWORD_CAST | KEYWORD_UPCAST | KEYWORD_DOWNCAST) WS* LPAREN WS* AT? IDENTIFIER WS* ASTERISK? WS* RPAREN WS* (
	raw_rvalue
	| string
	| singlequote_string
	| subshell
	| deprecated_subshell
	| object_reference
	| self_reference
	| pointer_dereference
	| object_address
	| supershell
	| new_statement);

array_value: ARRAY_ASSIGN_START statement* ARRAY_ASSIGN_END;

// Other statement
other_statement: ~(RBRACE | RBRACE_ROOTLEVEL | SUPERSHELL_END | QUOTE_END | SINGLEQUOTE_END | NEWLINE | SUBSHELL_END | DEPRECATED_SUBSHELL_END | BASH_ARITH_END | ARRAY_ASSIGN_END)+?;

// This rule will *only* ever be matched as part a value_assignment
raw_rvalue: IDENTIFIER | NUMBER | BASH_VAR;

extra_statement: RBRACE;
