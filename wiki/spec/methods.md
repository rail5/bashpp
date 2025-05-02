---
layout: custom
---
# NAME

@method - Define a method for a class

# SYNOPSIS

```bash
[@virtual] {@private | @protected | @public} @method {METHOD-NAME} [{ARGUMENTS}] {
	[COMMANDS]
}
```

# DESCRIPTION

The `@method` directive is used to define a method for a class. A method is a function that operates on the data members of the class. It can be declared as `@private`, `@protected`, or `@public`. Methods can also be declared as `@virtual`, which means that they can be overridden in a derived class.

The method name must be a valid Bash++ identifier. This means that it can only contain letters, numbers, and underscores, cannot start with a number, cannot be a reserved word, and cannot contain two consecutive underscores. The method name is case-sensitive. The method name must be unique within the class. If a method with the same name already exists in the class, an error will be generated.

The arguments to the method are optional.

If no arguments are specified, any arguments which are passed can still be accessed using `$1`, `$2`, etc, as in any other Bash function.

Methods cannot accept non-primitive arguments. They can, however, accept *pointers* to objects. For example:

```bash
@public @method myMethod @MyClass* myObject {
	if [[ @myObject != @nullptr ]]; then
		echo "@myObject.dataMember"
	fi
}
```

Declaring a pointer as an argument will run an implicit `@dynamic_cast` on the pointer to the expected type. This means that if the pointer is invalid, it will be set to `@nullptr` within the method.

Primitives can be declared as arguments as follows:

```bash
@public @method myMethod argument1 argument2 {
	echo "$argument1 $argument2"
}
```

The arguments can be accessed using `$1`, `$2`, etc, as in any other Bash function.

Declaring a method as `@virtual` means that it can be overridden in a derived class. This means that if a derived class has a method with the same name, the derived class's method will be called instead of the base class's method. The correct method to call is determined at runtime.

# EXAMPLE

```bash
@virtual @public @method myMethod argument1 @Object* myObject argument3 {
	if [[ @myObject != @nullptr ]]; then
		echo "$argument1 @myObject.dataMember $argument3"
	fi

	if [[ "$argument1" == "$1" ]]; then
		echo "\$argument1 is equal to \$1"
	fi

	if [[ "@myObject" == "$2" ]]; then
		echo "The pointer passed is equal to \$argument2"
	fi

	# Etc
}
```

# NOTES

Methods can only be defined within a class. They cannot be defined outside of a class.

Methods can be called using the `@object.method` syntax. For example, if you have an object called `myObject` and a method called `myMethod`, you can call the method using `@myObject.myMethod`.

Calling an object's method in a place where a primitive is expected will run the method in a supershell and substitute its output. For example:

```bash
echo @myObject.myMethod
```

Will run the method in a supershell, and then pass its output to the `echo` command.

This is equivalent to:

```bash
var=@(@myObject.myMethod)
echo $var
```

Referencing a non-primitive object directly in a place where a primitive is expected will run the `toPrimitive` method of the object. This means that the following two lines are equivalent:

```bash
echo @myObject
echo @myObject.toPrimitive
```

The toPrimitive method however will *not* be called if the reference is a **pointer** to an object, because pointers are already primitives. In that case, the pointer will be passed directly to the command.

# SEE ALSO

 - [bpp-classes(3)](classes.md) for more information on classes
 - [bpp-toprimitive(3)](toprimitive.md) for more information on the `toPrimitive` method
 - [bpp-supershell(3)](supershell.md) for more information on supershells
 - [bpp-dynamic-cast(3)](dynamic-cast.md) for more information on dynamic casting
 - [bpp-pointers(3)](pointers.md) for more information on pointers
