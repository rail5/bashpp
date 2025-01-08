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
DOLLAR: '$';

// Whitespace
WS: [ \t\r]+;

// Delimiters (newlines and semicolons, as in Bash)
DELIM: NEWLINE | SEMICOLON;

NEWLINE: '\n';
SEMICOLON: ';';

// Comments
COMMENT: '#' ~[\n]* -> skip;

// Strings
STRING: DOUBLEQUOTE_STRING | SINGLEQUOTE_STRING;

// Double-quoted strings
DOUBLEQUOTE_STRING: '"' (ESCAPE . | ~["\\])* '"';

// Single-quoted strings
SINGLEQUOTE_STRING: '\'' ~[']* '\'';

// Keywords
KEYWORD_CLASS: 'class';
KEYWORD_PUBLIC: 'public';
KEYWORD_PRIVATE: 'private';
KEYWORD_PROTECTED: 'protected';
KEYWORD_VIRTUAL: 'virtual';
KEYWORD_METHOD: 'method';
KEYWORD_CONSTRUCTOR: 'constructor';
KEYWORD_DESTRUCTOR: 'destructor';
KEYWORD_NEW: 'new';
KEYWORD_DELETE: 'delete';
KEYWORD_NULLPTR: 'nullptr';
KEYWORD_THIS: 'this';
KEYWORD_INCLUDE: 'include';

// Identifiers
IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]*;

// Bash variables
BASH_VAR: '$' IDENTIFIER
		| '$' '{' IDENTIFIER '}';

// Bash subshells
BASH_SUBSHELL: '$' '(' .+? ')';

// Bash arithmetic
BASH_ARITH: '$' '(' '(' .+? ')' ')';

// Operators
ASSIGN: '=';
DOT: '.';

LBRACE: '{';
RBRACE: '}';
LPAREN: '(';
RPAREN: ')';
LBRACKET: '[';
RBRACKET: ']';

ASTERISK: '*';
AMPERSAND: '&';

NUMBER: INTEGER | FLOAT;

INTEGER: [0-9]+;
FLOAT: [0-9]+ '.' [0-9]+;

MINUS: '-';
PLUS: '+';
SLASH: '/';
EXCLAM: '!';
COLON: ':';
COMMA: ',';
QUESTION: '?';
TILDE: '~';
CARET: '^';
PERCENT: '%';
LESSTHAN: '<';
GREATERTHAN: '>';
