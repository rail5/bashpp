# Language Specification

Manuals for various features of the Bash++ language are provided in this section. The language specification is a work in progress, and is not yet complete. If you have any questions or suggestions, please feel free to open an issue on the [GitHub repository](https://github.com/rail5/bashpp).

These manuals are also available as part of the Bash++ compiler and can be accessed via `man` commands. For example, to view the manual for the `@include` directive, run:

```bash
man bpp-include
```

## Contents

{% assign spec_pages = site.pages | where_exp: "page", "page.path contains 'spec/'" %}
<ul>
	{% for page in spec_pages %}
		{% unless page.path == 'spec/index.md' %}
			<li><a href="{{ page.url }}">bpp-{{ page.path | remove: 'spec/' | remove: '.md' }}(3)</a></li>
		{% endunless %}
	{% endfor %}
</ul>
