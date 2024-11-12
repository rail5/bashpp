parser grammar BashppParser;

options {tokenVocab=BashppLexer;}

// Here, we will define the grammar for Bash++
// We will assume that any statement which is not valid Bash++ is valid Bash

// Program
program: statement*;

// Statements
statement: class_definition | class_body_statement | object_instantiation | object_assignment | object_reference | open_brace | close_brace | other_statement;

// Class definition
class_definition: KEYWORD_CLASS IDENTIFIER open_brace statement*? close_brace;

// Class body statement
class_body_statement: (KEYWORD_PUBLIC | KEYWORD_PRIVATE) (variable_definition | method_definition | constructor_definition);

// Variable definition
variable_definition: BASHPP_VARIABLE? IDENTIFIER ((ASSIGN) (STRING | INTEGER | FLOAT | BOOLEAN | BASHPP_VARIABLE | PRIMITIVE_VARIABLE))?;

// Method definition
method_definition: KEYWORD_METHOD IDENTIFIER parameter_list? open_brace statement*? close_brace;

// Constructor definition
constructor_definition: KEYWORD_CONSTRUCTOR parameter_list? open_brace statement*? close_brace;

// Parameter list
parameter_list: (BASHPP_VARIABLE IDENTIFIER | IDENTIFIER)+;

// Object instantiation
object_instantiation: BASHPP_VARIABLE IDENTIFIER;

// Object assignment
object_assignment: object_reference ASSIGN (STRING | INTEGER | FLOAT | BOOLEAN | BASHPP_VARIABLE | PRIMITIVE_VARIABLE);

// Object reference
object_reference: BASHPP_VARIABLE (DOT IDENTIFIER)* argument_list?;

// Argument list
argument_list: (STRING | INTEGER | FLOAT | BOOLEAN | BASHPP_VARIABLE | PRIMITIVE_VARIABLE)+;

// Open brace
open_brace: LBRACE;

// Close brace
close_brace: RBRACE;

// Other statement
other_statement: .+?;
