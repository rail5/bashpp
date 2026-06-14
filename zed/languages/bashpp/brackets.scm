("(" @open
  ")" @close)

("[" @open
  "]" @close)

("{" @open
  "}" @close)

(("\"" @open
  "\"" @close)
  (#set! rainbow.exclude))

(("`" @open
  "`" @close)
  (#set! rainbow.exclude))

(("do" @open
  "done" @close)
  (#set! newline.only)
  (#set! rainbow.exclude))

((case_statement
  ("in" @open
    "esac" @close))
  (#set! newline.only)
  (#set! rainbow.exclude))

((if_statement
  ("then" @open
    "fi" @close))
  (#set! newline.only)
  (#set! rainbow.exclude))
