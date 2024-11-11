lexer grammar BashppLexer;

// Here, we will *only* define tokens used by Bash++
// The parser will assume that any statement which is not valid Bash++ is valid Bash
// This is because Bash++ is a superset of Bash
// This is probably not the best way to do this, but it's the easiest way to do it (for now)
// Why is it easier? Because we don't have to define every single token in Bash
// Especially since Bash is a very complex language, and there isn't an official "language spec" for Bash
// The biggest drawback to this approach is that it greatly limits the ability to provide useful error messages

// Tokens

AT: '@';
ESCAPE: '\\';

// Whitespace
WS: [ \t\r]+ -> skip;

// Delimiters (newlines and semicolons, as in Bash)
DELIM: [\n;];

// Comments
COMMENT: '#' ~[\n]* -> skip;

// Strings
STRING: DOUBLEQUOTE_STRING | SINGLEQUOTE_STRING;

// Double-quoted strings
DOUBLEQUOTE_STRING: '"' (ESCAPE . | ~["\\])* '"';

// Single-quoted strings
SINGLEQUOTE_STRING: '\'' ~[']* '\'';


// Keywords
KEYWORD_CLASS: '@class';
KEYWORD_PUBLIC: '@public';
KEYWORD_PRIVATE: '@private';
KEYWORD_METHOD: '@method';
KEYWORD_CONSTRUCTOR: '@constructor';

// Identifiers
IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]*;

// Operators
ASSIGN: '=';
DOT: '.';

// Variables
BASHPP_VARIABLE: '@' IDENTIFIER;
PRIMITIVE_VARIABLE: '$' IDENTIFIER;

// Literals
INTEGER: [0-9]+;
FLOAT: [0-9]+'.'[0-9]+;
BOOLEAN: 'true' | 'false';

LBRACE: '{';
RBRACE: '}';
