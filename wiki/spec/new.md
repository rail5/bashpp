# NAME

@new - Create a new instance of a class

# SYNOPSIS

```bash
@new {CLASS-NAME}
```

# DESCRIPTION

The `@new` directive is used to create a new instance of a class.

It initializes the object, calls its constructor (if it exists), and then returns a pointer to the object.

The output of the `@new` directive is a pointer to the newly created object.

# EXAMPLE

```bash
@MyClass* myObject=@new MyClass

shell_variable=@new MyClass

echo @new MyClass
```

# NOTES

Objects created with `@new` are not automatically deleted when they go out of scope. Unlike objects which are instantiated by `@ClassName objectName`, objects created with `@new` must be explicitly deleted using the `@delete` directive.

The object's constructor will be called *before* the pointer is returned. This means that the object will be fully initialized before anything can use it.

# SEE ALSO

- [bpp-delete(3)](delete.md) for deleting an object
- [bpp-classes(3)](classes.md) for more information on classes
