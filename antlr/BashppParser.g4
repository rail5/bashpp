parser grammar BashppParser;

options {tokenVocab=BashppLexer;}

// Here, we will define the grammar for Bash++
// We will assume that any statement which is not valid Bash++ is valid Bash

// Program
program: statement*;

// Statements
statement: (class_definition
		| class_body_statement
		| general_statement) DELIM?
		| DELIM
		| WS;

class_body_statement: member_declaration
					| method_definition
					| constructor_definition
					| destructor_definition
					| DELIM
					| WS;

general_statement: include_statement
				| object_instantiation
				| object_assignment
				| object_reference
				| self_reference
				| delete_statement
				| supershell
				| other_statement
				| DELIM
				| WS;

// Include statement
include_statement: AT KEYWORD_INCLUDE WS* STRING;

// Class definition
class_definition: AT KEYWORD_CLASS WS* IDENTIFIER WS* (COLON WS* IDENTIFIER WS*)? LBRACE (class_body_statement | general_statement)* RBRACE;

// Member declarations
member_declaration: AT (KEYWORD_PUBLIC | KEYWORD_PRIVATE | KEYWORD_PROTECTED) WS* IDENTIFIER value_assignment?
				| AT (KEYWORD_PUBLIC | KEYWORD_PRIVATE | KEYWORD_PROTECTED) WS* AT IDENTIFIER ASTERISK? WS* IDENTIFIER value_assignment?;

// Method definitions
method_definition: (AT KEYWORD_VIRTUAL WS*)? AT (KEYWORD_PUBLIC | KEYWORD_PRIVATE | KEYWORD_PROTECTED) WS* AT KEYWORD_METHOD WS* IDENTIFIER WS* parameter* WS* LBRACE (general_statement | class_body_statement)* RBRACE;

// Constructor definitions
constructor_definition: AT KEYWORD_CONSTRUCTOR WS* LBRACE general_statement* RBRACE;

// Destructor definitions
destructor_definition: AT KEYWORD_DESTRUCTOR WS* LBRACE general_statement* RBRACE;

// Value assignment
value_assignment: ASSIGN acceptable_rvalue?;

// Object instantiation
object_instantiation: AT IDENTIFIER WS* IDENTIFIER (value_assignment)?
				| AT IDENTIFIER ASTERISK WS* IDENTIFIER (value_assignment)?;

// Object assignment
object_assignment: (object_reference | self_reference) value_assignment;

// Pointer dereference
pointer_dereference: ASTERISK (object_reference | self_reference);

// Object address
object_address: AMPERSAND (object_reference | self_reference);

// Object reference
object_reference: AT IDENTIFIER (DOT IDENTIFIER)*;

// Self-reference from within a class
self_reference: AT KEYWORD_THIS (DOT IDENTIFIER)*;

// Delete statement
delete_statement: AT KEYWORD_DELETE WS* (object_reference | self_reference);

// Supershells
supershell: AT LPAREN statement* RPAREN;

parameter: IDENTIFIER | AT IDENTIFIER WS* IDENTIFIER;

acceptable_rvalue: IDENTIFIER
				| STRING
				| NUMBER
				| BASH_VAR
				| BASH_SUBSHELL
				| BASH_ARITH
				| object_reference
				| self_reference
				| nullptr_ref
				| new_statement
				| pointer_dereference
				| object_address;

nullptr_ref: AT KEYWORD_NULLPTR;

new_statement: AT KEYWORD_NEW WS* IDENTIFIER;

// Other statement
other_statement: ~'}'+?;
