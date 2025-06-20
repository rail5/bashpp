# Bash++ Standard Library

The Bash++ Standard Library is a collection of common data structures and algorithms implemented in Bash++. It is provided by the `libstd-bpp` package, which is included with the Bash++ compiler.

## Contents

{% assign stl_pages = site.pages | where_exp: "page", "page.path contains 'stl/'" %}
<ul>
	{% for page in stl_pages %}
		{% unless page.path == 'stl/index.md' %}
			<li><a href="{{ page.url }}">{{ page.title }}</a></li>
		{% endunless %}
	{% endfor %}
</ul>
