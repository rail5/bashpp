[
  (string)
  (raw_string)
  (heredoc_body)
  (heredoc_start)
  (heredoc_end)
  (ansi_c_string)
] @string

; TODO: HACK. Remove this override if the shared Bash grammar can recover
; cleanly from supershells containing positional expansions. It currently
; treats the rest of the command as raw strings, coloring unrelated Bash++
; code as string contents.
((command
  (variable_assignment
    value: (concatenation
      (word) @supershell
      (_)*
      (raw_string) @variable)))
  (#eq? @supershell "@")
  (#match? @variable "\n"))

((command
  (variable_assignment
    value: (concatenation
      (word) @supershell))
  name: (command_name
    (concatenation
      (_)*
      (raw_string) @variable)))
  (#eq? @supershell "@")
  (#match? @variable "\n"))

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
