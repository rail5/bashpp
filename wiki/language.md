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

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/classes-overview.html -%}
</code></pre></div>

## But what if I want to use the `@` symbol for something else?

You can simply *escape* the `@` symbol using a backslash:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/email-1.html -%}
</code></pre></div>

Or you can enclose it in *single-quotes*. As in Bash, single-quotes prevent the shell from interpreting the enclosed text, and we'll pass it through verbatim:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/email-2.html -%}
</code></pre></div>

# Data Members

Data members can be declared using the `@private`, `@public` or `@protected` keywords. If a data member is declared as `@public`, it can be accessed from outside the class. If it is declared as `@private`, it can only be accessed from within the class. If it is declared as `@protected`, it can be accessed from within the class and from derived classes.

Data members may be given default values, as shown in the example above. If no default value is provided, the data member will be initialized to an empty string.

A class can also contain *non-primitive* data members -- that is, objects.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/datamembers-example.html -%}
</code></pre></div>

Bash++ does not support nested classes.

We can access the data members of an object using the `@` symbol and the familiar `.` notation:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/datamembers-access.html -%}
</code></pre></div>

# Methods

Methods are the functions associated with the class; they're declared using the `@method` keyword. Methods can access both public and private data members of the class. A method can be declared as `@public`, `@private` or `@protected`, just like data members.

Methods can also take arguments:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/methods-example.html -%}
</code></pre></div>

Methods cannot take non-primitive arguments. They can, however, take *pointers* to objects as arguments:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/methods-pointer-arguments-example.html -%}
</code></pre></div>

Like in ordinary Bash functions, these arguments can also be accessed using `$1`, `$2`, etc.:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/methods-accessing-arguments-by-positional-variables.html -%}
</code></pre></div>

If a method is declared as `@virtual`, it can be overridden in derived classes. If a method is not declared as `@virtual`, it cannot be overridden.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/method-overriding-example.html -%}
</code></pre></div>

Bash++ does not support method overloading. Each method must have a unique name.

# Constructors and Destructors

Classes can have constructors and destructors, which are special methods that are called when an object is created and destroyed, respectively. Constructors are declared using the `@constructor` keyword, and destructors are declared using the `@destructor` keyword.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/constructors-destructors-example.html -%}
</code></pre></div>

An object's constructor will be called as soon as it is initialized.

An object's destructor will be called if:

 - The object goes out of scope before the script ends (e.g., the object is local to a method)

 - The object is explicitly destroyed using the `@delete` keyword

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/destructor-example-2.html -%}
</code></pre></div>

# .toPrimitive

In Bash++, every class has a method called `toPrimitive` that returns a string representation of the object. This method is called automatically when an object is used in a context where a primitive is expected.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/toprimitive-example.html -%}
</code></pre></div>

In the above example, the *default* `toPrimitive` method is used, and the script will output: `Object: MyClass Instance`.

You can define a custom `toPrimitive` method for your class to return a more meaningful string representation:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/custom-toprimitive-example.html -%}
</code></pre></div>

In this case, the script will output: `Object: MyClass instance with data member: Hello, world!`.

# Object References

When referencing an object, you use the `@` symbol followed by the object's name. This is similar to how you would reference a variable in Bash.

To reference an object's data member or method, you use the `@` symbol followed by the object's name, a dot (`.`), and the data member or method name.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/object-reference-example.html -%}
</code></pre></div>

There may be ambiguity when using the `@` symbol in a string. To avoid this, you can use the `@{}` syntax to explicitly reference an object. This is similar to the `${}` syntax used in Bash to reference variables in situations where there may be ambiguity. For example, suppose we want to echo the data member of an object:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/object-reference-ambiguity-1.html -%}
</code></pre></div>

This will unambiguously reference `dataMember` belonging to `myObject`. However, suppose what we *really* wanted to do was print the result of `myObject.toPrimitive` followed by the **string** `.dataMember`. We can use the `@{}` syntax to do this:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/object-reference-ambiguity-2.html -%}
</code></pre></div>

Here, we used the `@{}` syntax to explicitly reference `myObject` as an object, without interpreting the characters that followed it as part of the object reference.

This is identical to the `${}` syntax used in Bash to reference variables in situations where there may otherwise be ambiguity.

# `@this` Keyword

The `@this` keyword is used to refer to the current object within a method. It is similar to the `this` keyword in other object-oriented languages.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/this-example.html -%}
</code></pre></div>

# Inheritance

Bash++ supports single inheritance. A class can inherit from another class using the following syntax:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/inheritance-example.html -%}
</code></pre></div>

In this example, `DerivedClass` inherits from `BaseClass`. The derived class has access to the data members and methods of the base class. If a method is declared `@virtual`, it can be overridden in the derived class. If a method is not declared `@virtual`, it cannot be overridden.

# Explicit Object Casting

Bash++ supports explicit object casting using the `@cast` keyword. This allows you to cast an object to a different class.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/object-casting-example.html -%}
</code></pre></div>

The `@cast` keyword does not perform any type checking. It simply changes the type of the object. If the cast is invalid, the object will be in an invalid state, but the compiler will not throw an error.

For *slightly* more careful casting, you can use the `@upcast` or `@downcast` keywords. `@upcast` will cast an object to a base class, while `@downcast` will cast an object to a derived class. If the cast is invalid, the compiler will throw an error.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/upcast-downcast-example.html -%}
</code></pre></div>

An `@upcast` will perform a compile-time check to verify that we're casting *up* the inheritance hierarchy. A `@downcast` will perform a similar check to verify that we're casting *down* the inheritance hierarchy. If the cast is invalid, the compiler will throw an error.

You can also cast pointers:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/pointer-casting-example.html -%}
</code></pre></div>

# Pointers

Bash++ supports pointers to objects. A pointer is a reference to an object, rather than the object itself. Pointers are declared using the familiar C-style `*` syntax:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/pointers-example.html -%}
</code></pre></div>

In the above example, `myPointer` is a pointer to an object of type `MyClass`. It is initially set to nothing at all (`@nullptr`). The pointer is then assigned to `myObject`. Afterwards, a new object is created using the `@new` keyword, and a pointer to it is stored in `myObject2`.

We can access the data members of a pointer using exactly the same syntax as we would for an object:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/object-reference-via-pointer.html -%}
</code></pre></div>

We don't have to use the spooky `->` operator from C++. Just use the familiar `.` notation.

Pointers themselves are considered primitives. Unfortunately, we're going to spend a moment discussing some of the *internals* of Bash++ to explain why this is the case. This is not ideal for a document describing how to *use* the language.

## How objects are stored in Bash++

Since Bash++ compiles to Bash, we don't have direct, unadulterated access to memory. We can't just allocate a block of memory and store our objects there. Instead, we have to use Bash's built-in data structures to store our objects.

Bash++ keeps track of objects internally by assigning values to variables with special names. For example, the following:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/how-objects-are-stored-1.html -%}
</code></pre></div>

Might be internally represented as:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/how-objects-are-stored-2.html -%}
</code></pre></div>

When we create a pointer to an object, or ask about this object's "address," we're really just getting the *prefix* of the variable names that represent the object. In the above case, the address given by `&@myObject` would be `bpp__MyClass__myObject`.

This is why pointers are considered primitives -- they're just strings that represent the "address" of an object.

We mentioned at the beginning of this document that identifiers cannot contain two consecutive underscores. This is because, as you can see in the above example, Bash++ uses double underscores to separate class names from data members and methods **internally**. Allowing double underscores in identifiers would risk collisions between user-defined identifiers and the internal identifiers automatically generated by the compiler, leading to lots of ambiguity and unexpected behavior.

Because pointers are primitives, using a pointer in a context where a primitive would be expected will not call the `toPrimitive` method automatically, but will simply return the pointer. However, we can still call the `toPrimitive` method explicitly:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/pointers-and-toprimitive.html -%}
</code></pre></div>

## `@new` and `@delete`

The `@new` keyword is used to create a new object and return a pointer to it. The `@delete` keyword is used to destroy an object and set the pointer to `@nullptr`.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/new-and-delete.html -%}
</code></pre></div>

An object's destructor will be called when it is deleted.

We can also declare pointers as data members of a class:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/pointers-as-datamembers.html -%}
</code></pre></div>

A class's destructor will not automatically delete its pointer data members. If you want to delete a pointer data member, you must do so explicitly. If, however, a class contains non-primitive data members (which are not pointers), they will be automatically deleted when the object is destroyed.

All pointers are set to `@nullptr` by default until otherwise specified. For example:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/nullptr.html -%}
</code></pre></div>

When writing `@new {some class}`, or `@delete {some object/pointer}`, the identifier which follows the `@new` or `@delete` keyword must be a class name or an object/pointer, respectively. It can be optionally preceded by an `@` symbol, but this is not required. `@new @MyClass` and `@new MyClass` are equivalent. `@delete @myObject` and `@delete myObject` are equivalent.

## `@nullptr`

`@nullptr` is a special value that represents a null pointer. It is used to indicate that a pointer does not point to any object.

If a pointer is set to `@nullptr`, attempting to access a data member or method of the object it points to will result in an error.

If a pointer is declared without being assigned a value, it is automatically set to `@nullptr`.

# `@include` Directive

The `@include` directive is used to include the contents of another Bash++ script in the current file. This is useful for splitting code into multiple files for better organization and reusability.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/include-example.html -%}
</code></pre></div>

The `@include` directive is processed at compile time, and the contents of the included file are inserted into the current file before compilation.

Note that you can still include regular Bash scripts in a Bash++ script using the `source` command (or `.`):

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/source.html -%}
</code></pre></div>

However, scripts included with `source` or `.` will not be processed by the Bash++ compiler and will not have access to Bash++ features.

You can also use the `@include_once` directive to ensure that a file is included only once. This can be useful if you have multiple files that include the same file.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/include-once-example.html -%}
</code></pre></div>

# Supershells

Ordinary Bash supports a construct called a "subshell," which is created by enclosing a command in parentheses. A subshell is a separate instance of the shell that runs the commands within it. When the subshell exits, any changes to the environment made within it are lost.

Subshells are generally useful when you want to run a command in a separate environment or when you want to isolate changes to the environment. But, another common use for subshells is simply to store the output of a command in a variable:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/subshells.html -%}
</code></pre></div>

This is perfectly reasonable. However, with Bash++ methods, we may like to store the output of a method in a variable, while also *allowing* the method to make changes to the environment. This is where the "supershell" comes in.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/supershells.html -%}
</code></pre></div>

As you can see from the above, the syntax is very similar. The only difference is the use of the `@` symbol instead of the `$`. This allows us to store the output of a command, function, or method into a variable, while also preserving changes that that command, function, or method makes to the environment.

By default, calling an object's method in a context where a primitive is expected will run the method inside of a supershell. For example:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/supershells-as-default.html -%}
</code></pre></div>

The above will run `@myObject.myMethod` in a *supershell*, allowing it to make changes to the environment. The output of the method will be stored in `command_output`, and the value of `dataMember` will be changed to "New value."

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/supershells-as-default-2.html -%}
</code></pre></div>

If, by contrast, we were to run the method in an ordinary *subshell*:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/subshells-2.html -%}
</code></pre></div>

The output of the command would be stored, but `@myObject.dataMember` would not be changed.

You can also run ordinary commands and Bash functions in supershells, allowing them to make changes to the environment as well. For example:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/supershells-ordinary-bash-functions.html -%}
</code></pre></div>

If you would like to isolate an object's method from the surrounding environment, you can still call it within a subshell:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/object-method-within-a-subshell.html -%}
</code></pre></div>

Supershells *do not* run in an isolated environment or separate shell. They run in the same shell as the surrounding code -- their output is simply captured and stored rather than printed.

You can also run a command in a supershell without storing its output in a variable:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/supershells-without-permanent-storage.html -%}
</code></pre></div>
