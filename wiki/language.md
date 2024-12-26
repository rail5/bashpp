---
layout: custom
title: Programming in Bash++
---

# Programming in Bash++

Bash++ is a superset of Bash that adds support for object-oriented programming. Almost all Bash code is valid Bash++ code.

# Identifiers

The rules for identifiers in Bash++ are almost identical to those in Bash.

In Bash, an identifier is a sequence of letters, digits, or underscores that begins with a letter or underscore. Identifiers are case-sensitive.

In Bash++, the same rules apply, with the small and somewhat inconvenient exception that identifiers cannot contain two consecutive underscores. This is because of how Bash++ works internally (see the section on pointers for more information). The takeaway is that `my_object`, `_myObject`, and `_my_Object_` are all valid identifiers, but `my__object`, `my______object`, and `_________` are not.

# Classes

## Introduction

In Bash++, there are two categories of types:

 - **Primitives**

 - **Objects**

Primitives are the regular-old shell variables that you're used to. You can use them in exactly the same way that you normally do in Bash. Strings, integers, arrays, and associative arrays are all examples of primitives.

Objects are instances of classes, which are user-defined types that can contain data members and methods.

To distinguish between the two, Bash++ uses the `@` symbol to denote objects. For example, `@myObject` refers to an object, while `$myVariable` refers to a primitive.

## Overview

Classes are declared in Bash++ using the `@class` keyword. The class definition can contain data members, methods, and constructors/destructors.

```bash
@class MyClass {
	@private dataMember1
	@public dataMember2="default value"

	@public @method myMethod {
		echo "Hello from myMethod"
	}

	@constructor {
		echo "Constructor called"
	}

	@destructor {
		echo "Destructor called"
	}
}
```

## But what if I want to use the `@` symbol for something else?

You can simply *escape* the `@` symbol using a backslash:

```bash
echo "email\@address.com"
```

# Data Members

Data members can be declared using the `@private`, `@public` or `@protected` keywords. If a data member is declared as `@public`, it can be accessed from outside the class. If it is declared as `@private`, it can only be accessed from within the class. If it is declared as `@protected`, it can be accessed from within the class and from derived classes.

Data members may be given default values, as shown in the example above. If no default value is provided, the data member will be initialized to an empty string.

A class can also contain *non-primitive* data members -- that is, objects.

```bash
@class Object {
	@public dataMember
}

@class MyClass {
	@public primitiveDataMember
	@public @Object nonPrimitiveDataMember
}
```

Bash++ does not support nested classes.

We can access the data members of an object using the `@` symbol and the familiar `.` notation:

```bash
@MyClass myObject

@myObject.primitiveDataMember="Hello, world!"
echo "@myObject.primitiveDataMember"

@myObject.nonPrimitiveDataMember.dataMember="Hello, world!"
echo "@myObject.nonPrimitiveDataMember.dataMember"
```

# Methods

Methods are the functions associated with the class; they're declared using the `@method` keyword. Methods can access both public and private data members of the class. A method can be declared as `@public`, `@private` or `@protected`, just like data members.

Methods can also take arguments:

```bash
@class MyClass {
	@public @method myMethod {
		echo "Hello from myMethod"
	}

	@public @method myMethodWithArgs arg1 arg2 {
		echo "Hello from myMethodWithArgs"
		echo "Argument 1: $arg1"
		echo "Argument 2: $arg2"
	}
}

@MyClass myObject
@myObject.myMethod
@myObject.myMethodWithArgs "Hello" "World"
```

Further, methods can take *objects* as arguments:

```bash
@class Object {
	@public dataMember
}

@class MyClass {
	@public @method myMethodWithObject @Object obj {
		echo "Hello from myMethodWithObject"
		echo "Object data member: @obj.dataMember"
	}
}
```

# Constructors and Destructors

Classes can have constructors and destructors, which are special methods that are called when an object is created and destroyed, respectively. Constructors are declared using the `@constructor` keyword, and destructors are declared using the `@destructor` keyword.

```bash
@class MyClass {
	@constructor {
		echo "Constructor called"
	}
	@destructor {
		echo "Destructor called"
	}
}
```

An object's constructor will be called as soon as it is initialized.

An object's destructor will be called if:

 - The object goes out of scope before the script ends (e.g., the object is local to a method)

 - The object is explicitly destroyed using the `@delete` keyword

```bash
@class MyClass {
	@destructor {
		echo "Destructor called"
	}
}
@MyClass myObject
@delete myObject
```

# "toPrimitive" Object Casting

In Bash++, every class has a method called `toPrimitive` that returns a string representation of the object. This method is called automatically when an object is used in a context where a primitive is expected.

```bash
@class MyClass {
	@public dataMember="Hello, world!"
	# No custom toPrimitive method is defined here
}

@MyClass myObject

echo "Object: @myObject" # Calls myObject.toPrimitive
```

In the above example, the *default* `toPrimitive` method is used, and the script will output: `Object: MyClass Instance`.

You can define a custom `toPrimitive` method for your class to return a more meaningful string representation:

```bash
@class MyClass {
	@public dataMember="Hello, world!"

	@public @method toPrimitive {
		echo "MyClass instance with data member: @this.dataMember"
	}
}

@MyClass myObject

echo "Object: @myObject" # Calls myObject.toPrimitive
```

In this case, the script will output: `Object: MyClass instance with data member: Hello, world!`.

# Object References

When referencing an object, you use the `@` symbol followed by the object's name. This is similar to how you would reference a variable in Bash.

To reference an object's data member or method, you use the `@` symbol followed by the object's name, a dot (`.`), and the data member or method name.

```bash
@MyClass myObject
@myObject.dataMember="Hello, world!"
echo "@myObject.dataMember"
```

There may be ambiguity when using the `@` symbol in a string. To avoid this, you can use the `@{}` syntax to explicitly reference an object. This is similar to the `${}` syntax used in Bash to reference variables in situations where there may be ambiguity. For example, suppose we want to echo the data member of an object:

```bash
@class MyClass {
	@public dataMember="Data Member"
	@public @method toPrimitive {
		echo "object"
	}
}

@MyClass myObject
echo "@myObject.dataMember" # Prints "Data Member"
```

This will unambiguously reference `dataMember` belonging to `myObject`. However, suppose what we *really* wanted to do was print the result of `myObject.toPrimitive` followed by the **string** `.dataMember`. We can use the `@{}` syntax to do this:

```bash
echo "@{myObject}.dataMember" # Prints "object.dataMember"
```

Here, we used the `@{}` syntax to explicitly reference `myObject` as an object, without interpreting the characters that followed it as part of the object reference.

This is identical to the `${}` syntax used in Bash to reference variables in situations where there may otherwise be ambiguity.

# `@this` Keyword

The `@this` keyword is used to refer to the current object within a method. It is similar to the `this` keyword in other object-oriented languages.

```bash
@class MyClass {
	@public dataMember="Hello, world!"

	@public @method myMethod {
		echo "Data member: @this.dataMember"
	}
}
```

# Inheritance

Bash++ supports single inheritance. A class can inherit from another class using the following syntax:

```bash
@class BaseClass {
	@public baseDataMember="Hello, world!"

	@public @method someMethod {
		echo "Hello from the base class!"
	}
}

@class DerivedClass : BaseClass {
	@public derivedDataMember="Hello, world!"

	@public @method someMethod {
		echo "Hello from the derived class!"
	}
}
```

In this example, `DerivedClass` inherits from `BaseClass`. The derived class has access to the data members and methods of the base class. If a method is overridden in the derived class, the derived class's implementation will be used.

# Pointers

Bash++ supports pointers to objects. A pointer is a reference to an object, rather than the object itself. Pointers are declared using the familiar C-style `*` syntax:

```bash
@class MyClass {
	@public dataMember="Hello, world!"
}

@MyClass myObject
@MyClass* myPointer # Empty pointer
@myPointer=&@myObject # Assign pointer to object

@MyClass myObject2=@new @MyClass
```

In the above example, `myPointer` is a pointer to an object of type `MyClass`. It is initially set to nothing at all (`@nullptr`). The pointer is then assigned to `myObject`. Afterwards, a new object is created using the `@new` keyword, and a pointer to it is stored in `myObject2`.

Bash++ automatically (implicitly) dereferences pointers as needed. For example:

```bash
@myPointer.dataMember="Hello, world!"
echo "@myPointer.dataMember"
```

We don't have to explicitly dereference `myPointer` -- Bash++ does it for us. Nor do we have to use the spooky `->` operator from C++. Just use the familiar `.` notation.

Pointers themselves are considered primitives. Unfortunately, we're going to spend a moment discussing some of the *internals* of Bash++ to explain why this is the case. This is not ideal for a document describing how to *use* the language.

## How objects are stored in Bash++

Since Bash++ compiles to Bash, we don't have direct, unadulterated access to memory. We can't just allocate a block of memory and store our objects there. Instead, we have to use Bash's built-in data structures to store our objects.

Bash++ keeps track of objects internally by assigning values to variables with special names. For example, the following:

```bash
@class MyClass {
	@public dataMember="Hello, world!"
	@public anotherDataMember="Goodbye, world!"
}

@MyClass myObject
```

Might be internally represented as:

```bash
bpp__MyClass__myObject__dataMember="Hello, world!"
bpp__MyClass__myObject__anotherDataMember="Goodbye, world!"
```

When we create a pointer to an object, or ask about this object's "address," we're really just getting the *prefix* of the variable names that represent the object. In the above case, the address given by `&@myObject` would be `bpp__MyClass__myObject`.

This is why pointers are considered primitives -- they're just strings that represent the "address" of an object.

We mentioned at the beginning of this document that identifiers cannot contain two consecutive underscores. This is because, as you can see in the above example, Bash++ uses double underscores to separate class names from data members and methods **internally**. Allowing double underscores in identifiers would risk collisions between user-defined identifiers and the internal identifiers automatically generated by the compiler, leading to lots of ambiguity and unexpected behavior.

Because pointers are primitives, using a pointer in a context where a primitive would be expected will not call the `toPrimitive` method automatically, but will simply return the pointer. However, since pointers are implicitly dereferenced as needed, we can still call the `toPrimitive` method explicitly:

```bash
@MyClass* myPointer=@new @MyClass # Declare a pointer
echo "@myPointer" # Echoes the pointer
echo "@myPointer.toPrimitive" # Calls the toPrimitive method

@MyClass myObject # Declare an object (not a pointer)
echo "@myObject" # Calls the toPrimitive method
echo "@myObject.toPrimitive" # Also calls the toPrimitive method
```

## `@new` and `@delete`

The `@new` keyword is used to create a new object and return a pointer to it. The `@delete` keyword is used to destroy an object and set the pointer to `@nullptr`.

```bash
@class MyClass {
	@public dataMember="Hello, world!"
}

@MyClass* myPointer=@new @MyClass

@delete myPointer
```

An object's destructor will be called when it is deleted.

We can also declare pointers as data members of a class:

```bash
@class MyClass {
	@public @MyClass* pointer
}
```

A class's destructor will not automatically delete its pointer data members. If you want to delete a pointer data member, you must do so explicitly. If, however, a class contains non-primitive data members (which are not pointers), they will be automatically deleted when the object is destroyed.

All pointers are set to `@nullptr` by default until otherwise specified. For example:

```bash
@MyClass* myPointer1
@MyClass* myPointer2=@nullptr # Same result as above
@MyClass* myPointer3=@new @MyClass # Not @nullptr
```

When writing `@new {some class}`, or `@delete {some object/pointer}`, the identifier which follows the `@new` or `@delete` keyword must be a class name or an object/pointer, respectively. It can be optionally preceded by an `@` symbol, but this is not required. `@new @MyClass` and `@new MyClass` are equivalent. `@delete @myObject` and `@delete myObject` are equivalent.

## `@nullptr`

`@nullptr` is a special value that represents a null pointer. It is used to indicate that a pointer does not point to any object.

If a pointer is set to `@nullptr`, attempting to access a data member or method of the object it points to will result in an error.

If a pointer is declared without being assigned a value, it is automatically set to `@nullptr`.

# `@include` Directive

The `@include` directive is used to include the contents of another Bash++ script in the current file. This is useful for splitting code into multiple files for better organization and reusability.

```bash
@include "file1.bpp" # Same working directory
@include "/path/to/file.bpp" # Absolute path
```

The `@include` directive is processed at compile time, and the contents of the included file are inserted into the current file before compilation.

Note that you can still include regular Bash scripts in a Bash++ script using the `source` command (or `.`):

```bash
source "script.sh"
```

However, scripts included with `source` or `.` will not be processed by the Bash++ compiler and will not have access to Bash++ features.

# Supershells

Ordinary Bash supports a construct called a "subshell," which is created by enclosing a command in parentheses. A subshell is a separate instance of the shell that runs the commands within it. When the subshell exits, any changes to the environment made within it are lost.

Subshells are generally useful when you want to run a command in a separate environment or when you want to isolate changes to the environment. But, another common use for subshells is simply to store the output of a command in a variable:

```bash
var=$(command)
```

This is perfectly reasonable. However, with Bash++ methods, we may like to store the output of a method in a variable, while also *allowing* the method to make changes to the environment. This is where the "supershell" comes in.

```bash
var=@(command)
```

As you can see from the above, the syntax is very similar. The only difference is the use of the `@` symbol instead of the `$`. This allows us to store the output of a command, function, or method into a variable, while also preserving changes that that command, function, or method makes to the environment.

By default, calling an object's method in a context where a primitive is expected will run the method inside of a supershell. For example:

```bash
@class MyClass {
	@public dataMember="Default value"
	@public @method myMethod {
		@this.dataMember="New value"
		echo "Hello from myMethod"
	}
}

@MyClass myObject
command_output="@myObject.myMethod"
echo "$command_output"
```

The above will run `@myObject.myMethod` in a *supershell*, allowing it to make changes to the environment. The output of the method will be stored in `command_output`, and the value of `dataMember` will be changed to "New value."

```bash
command_output="@myObject.myMethod"
command_output=@(@myObject.myMethod) # These two are exactly equivalent,
                                     # both run in supershells and can
                                     # therefore make changes to the environment
```

If, by contrast, we were to run the method in an ordinary *subshell*:

```bash
command_output=$(@myObject.myMethod)
```

The output of the command would be stored, but `@myObject.dataMember` would not be changed.

You can also run ordinary commands and Bash functions in supershells, allowing them to make changes to the environment as well. For example:

```bash
regular_bash_variable="abc"

function regular_bash_function() {
	regular_bash_variable="123"
	echo "Hello from an ordinary Bash function"
}

# Using a Bash subshell:
echo "Initial value of regular_bash_variable: $regular_bash_variable" # "abc"
command_output=$(regular_bash_function)
echo "New value of regular_bash_variable: $regular_bash_variable" # Still "abc", unchanged
echo "Command output: $command_output" # "Hello from an ordinary Bash function"

# Using a Bash++ supershell:
echo "Initial value of regular_bash_variable: $regular_bash_variable" # "abc"
command_output=@(regular_bash_function)
echo "New value of regular_bash_variable: $regular_bash_variable" # "123"
echo "Command output: $command_output" # "Hello from an ordinary Bash function"
```

If you would like to isolate an object's method from the surrounding environment, you can still call it within a subshell:

```bash
command_output=$(@myObject.myMethod) # Ordinary subshell, won't change the environment
```
