[
  (string)
  (raw_string)
  (heredoc_body)
  (heredoc_start)
  (heredoc_end)
  (ansi_c_string)
] @string

(variable_name) @variable

[
  "export"
  "function"
  "unset"
  "local"
  "declare"
] @keyword

[
  "case"
  "do"
  "done"
  "elif"
  "else"
  "esac"
  "fi"
  "for"
  "if"
  "in"
  "select"
  "then"
  "until"
  "while"
] @keyword.control

(comment) @comment

((program
  .
  (comment) @keyword.directive)
  (#match? @keyword.directive "^#![ \t]*/"))

(function_definition
  name: (word) @function)

(command_name
  (word) @function)

[
  (file_descriptor)
  (number)
] @number

(regex) @string.regex

[
  (command_substitution)
  (process_substitution)
  (expansion)
] @embedded

[
  "$"
  "&&"
  "||"
  ">"
  "<<"
  ">>"
  ">&"
  ">&-"
  "<"
  "|"
  "="
  "=~"
  "=="
  "!="
  "-o"
  "-a"
  "+"
  "-"
  "*"
  "**"
  "!"
] @operator

(test_operator) @keyword.operator

";" @punctuation.delimiter

[
  "("
  ")"
  "{"
  "}"
  "["
  "]"
] @punctuation.bracket

(special_variable_name) @variable.special

((word) @keyword
  (#match? @keyword "^@(class|method|constructor|destructor|public|private|protected|virtual|new|delete|nullptr|include|include_once|this|super|typeof|dynamic_cast)$"))
