# Bash++ Standard Library

<link rel="stylesheet" href="{{ '/assets/css/card-style.css' | relative_url }}">

The Bash++ Standard Library is a collection of common data structures and algorithms implemented in Bash++. It is provided by the `libstd-bpp` package, which is included with the Bash++ compiler.

## Contents

<div class="cards-grid">
{% assign stl_pages = site.pages | where_exp: "page", "page.path contains 'stl/'" %}
	{% for page in stl_pages %}
		{% unless page.path == 'stl/index.md' %}
			{% assign includeDirective = '@include &lt;' | append: page.sourceFile | append: '&gt;' %}
			{% assign declaration = '@class ' | append: page.title %}
			{% if page.parent %}
				{% assign declaration = declaration | append: ' : ' | append: page.parent %}
			{% endif %}

			{% include card.html
				url=page.url
				title=page.title
				subtitle=includeDirective
				brief=declaration
				description=page.description
			%}
		{% endunless %}
	{% endfor %}
</div>
