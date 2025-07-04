---
layout: custom
title: Objects
---
# NAME

Objects - Object-Oriented Programming in Bash++

# SYNOPSIS

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/objects-example.html -%}
</code></pre></div>

# DESCRIPTION

Bash++ objects are instances of classes that encapsulate data and behavior. They are created from class definitions and can have their own state and methods. Objects are the fundamental building blocks of object-oriented programming in Bash++.

## Creating and deleting an object

When you create an object, Bash++ automatically calls the constructor of the class to initialize the object.

The destructor of the class is then called if:

 - The object goes out of scope **before the program exits**, and the object was not created via `@new`
 - The object is explicitly deleted using the `@delete` directive

Destructors are *never* called for objects created with `@new` until the object is explicitly deleted using `@delete`.

Destructors for **global-scope** objects are **not run at program termination**. <u>This is to avoid non-deterministic behavior in forked processes.</u> If you need to ensure that destructors are called for global-scope objects, you should call them explicitly using `@delete`.

An object "going out of scope *before* the program exits" means that the object is no longer accessible in the current context, i.e., the object was local to a function or method which has returned, or the object was created in a block that has ended.

## Accessing object methods and variables

To access methods and variables of an object, you can use the dot notation. For example, if you have an object `myObject` of class `MyClass`, you can access its method `myMethod` and variable `myVariable` like this:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/objects-example-2.html -%}
</code></pre></div>

The dot signals that we are descending the object hierarchy to access something internal to the object. For example, `myObject` can contain another non-primitive `innerObject`, and you can access its method `innerMethod` like this:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/objects-example-3.html -%}
</code></pre></div>

The notation for descending into an object's methods or variables is the same for both objects and pointers to objects. You do not need to use the spooky `->` operator in the case of pointers. Bash++ pointers are automatically dereferenced as needed.

Referencing an object in a place where a primitive is expected (e.g., in an `echo` command) will implicitly call the object's `toPrimitive` method. All classes in Bash++ have a `toPrimitive` method, which is used to convert the object to a primitive value. This method can be overridden to provide custom behavior.

# SEE ALSO
- [bpp-classes(3)](classes.md) for more information on classes
- [bpp-new(3)](new.md) for creating new instances of classes
- [bpp-delete(3)](delete.md) for deleting objects
- [bpp-methods(3)](methods.md) for more information on object methods
- [bpp-toprimitive(3)](toprimitive.md) for more information on the `toPrimitive` method
- [bpp-pointers(3)](pointers.md) for more information on pointers to objects
- [bpp-supershell(3)](supershell.md) for more information on supershells
