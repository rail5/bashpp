---
layout: custom
---
# NAME

toPrimitive - Convert an object to a primitive type

# SYNOPSIS

```bash
@object.toPrimitive
```

# DESCRIPTION

The `toPrimitive` method is a built-in method that is automatically available to all classes. It is used to convert an object to a primitive type.

The `toPrimitive` method is called when an object is used in a context where a primitive type is expected.

The method is automatically included in all classes. It is always **virtual** and always **public**.

A custom implementation can be provided in a class, but is not required. The default implementation will echo `{CLASS-NAME} Instance`. For example, if the class is `MyClass`, the default implementation will echo `MyClass Instance`.

# EXAMPLE

```bash

@class FileSystemDirectory {
	@private path="~/"

	@public @method setPath path {
		@this.path="$path"
	}

	@public @method toPrimitive {
		ls "@this.path"
	}
}

@FileSystemDirectory logDirectory
@logDirectory.setPath "/var/log"

echo @logDirectory # Implicitly calls the `toPrimitive` method
echo @logDirectory.toPrimitive # Equivalent
```

# NOTES

The `toPrimitive` method is automatically called when an object is used in a context where a primitive type is expected. This includes situations such as:

 - Assigning an object to a primitive variable
 - Using an object in a string context
 - Using an object in a mathematical operation
 - Using an object as an argument to a command
 - Etc

Your custom implementation of toPrimitive can also take arguments if desired, but this is not required.

# SEE ALSO

 - [bpp-methods(3)](methods.md) for more information on object methods
 - [bpp-classes(3)](classes.md) for more information on classes
