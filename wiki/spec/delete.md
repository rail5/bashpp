# NAME

@delete - Delete an object

# SYNOPSIS

```bash
@delete @{OBJECT | POINTER}
```

# DESCRIPTION

The `@delete` directive is used to delete an object.

It first calls the object's destructor (if it exists) and then deletes the object.

The input can be either a pointer to an object or an object itself. If the input is a pointer, the pointer will be set to `@nullptr` after the object is deleted.

The `@delete` directive gives no output unless there is an error. If the input to `@delete` is an invalid pointer, it will output an error message.

# EXAMPLE

```bash
@Object obj
@Object* objPtr=@new Object

@delete @obj
@delete @objPtr
```

# NOTES

All constructors and destructors are virtual. This means that calling `@delete` will always call the destructor of the correct type for the object, even if given a pointer of the wrong type. For example:

```bash
@Object obj

@UnrelatedClass* wrongPtrType=&@obj

@delete @wrongPtrType # This will call the destructor of Object, not UnrelatedClass
```

# SEE ALSO

 - [bpp-new(3)](new.md) for creating an object
 - [bpp-classes(3)](classes.md) for more information on classes
