# Language Specification

<link rel="stylesheet" href="{{ '/assets/css/card-style.css' | relative_url }}">

Manuals for various features of the Bash++ language are provided in this section. The language specification is a work in progress, and is not yet complete. If you have any questions or suggestions, please feel free to open an issue on the [GitHub repository](https://github.com/rail5/bashpp).

These manuals are also available as part of the Bash++ compiler and can be accessed via `man` commands. For example, to view the manual for the `@include` directive, run: `man bpp-include`.

## General

A Bash++ source file must be a valid UTF-8 encoded text file with Unix-style line endings (LF, `\n`). The file may optionally begin with a shebang line (e.g., `#!/usr/bin/env bpp`) to allow execution as a script. It is suggested to use the `.bpp` file extension for Bash++ source files, although not required.

In general, since Bash++ is a superset of Bash, the details of Bash syntax and semantics apply unless otherwise specified in the Bash++ documentation. All valid Bash scripts that do not contain unescaped AT symbols (`@`) are also valid Bash++ scripts with identical behavior. Any case in which there exists a deviation from Bash behavior that is not documented should be reported as a bug.

## Contents

<div class="cards-grid">
{% assign spec_pages = site.pages | where_exp: "page", "page.path contains 'spec/'" %}
	{% for page in spec_pages %}
		{% unless page.path == 'spec/index.md' %}
			{% assign slug = page.path | remove: 'spec/' | remove: '.md' %}
			{% assign manpage = 'bpp-' | append: slug | append: '(3)' %}

			{% include card.html
				url=page.url
				title=page.title
				subtitle=manpage
				brief=page.brief
				description=page.description
			%}
		{% endunless %}
	{% endfor %}
</div>
