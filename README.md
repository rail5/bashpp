# Bash++

Bash with classes

```javascript
@class Bashpp {
  @public author="Andrew S. Rightenburg"
  @public source="https://github.com/rail5/bashpp"
  @public license="GNU GPL v3"

  @public @method printInfo {
	echo "Bash++ is a superset of Bash that adds support for classes and objects."
	echo "It's designed to be a simple way to add object-orientation to Bash scripts."
	echo "Author: @this.author"
	echo "Source: @this.source"
	echo "License: @this.license"
  }
}

@Bashpp myBashpp
@myBashpp.printInfo
```

More documentation is available on the [website](https://bpp.sh).
