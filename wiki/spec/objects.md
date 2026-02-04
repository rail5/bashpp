---
layout: custom
title: Objects
brief: "Objects"
description: "Object-Oriented Programming in Bash++"
---
# NAME

Objects - Object-Oriented Programming in Bash++

# SYNOPSIS

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/objects-example.html -%}
</code></pre></div>

# DESCRIPTION

Objects are instances of classes. They are created from class definitions and can have their own state and methods.

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

The notation for descending into an object's methods or variables is the same for both objects and pointers to objects. You do not need to use a different operator in the case of pointers, as in many other languages. Bash++ pointers are automatically dereferenced as needed.

Referencing an object in a place where a primitive is expected (e.g., as an argument to a command) will implicitly call the object's `toPrimitive` method. All classes in Bash++ have a `toPrimitive` method, which is used to convert the object to a primitive value. This method can be overridden to provide custom behavior.

### Accessing array elements in object variables

In ordinary Bash, you can index arrays using the syntax `${array[index]}`. The syntax to index arrays which are data members in objects should be easily intuited:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/objects-example-4.html -%}
</code></pre></div>

One minor peculiarity is in *assigning* values to specific indices of a data member array. In ordinary Bash, when doing this, the array element becomes an *lvalue* reference and therefore must drop the `${}` encasement, as in `array[index]="value"`.

In Bash++, however, we cannot follow *quite* this same rule (dropping the entire `@{}` encasement) because the reference would then be syntactically indistinguishable from a primitive array assignment, and would be very difficult to parse with an LALR(1) parser. The rule therefore is to drop the `{}` encasement but retain the `@` sigil, as in:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/objects-example-5.html -%}
</code></pre></div>

# SEE ALSO
- [bpp-classes(3)](classes.md) for more information on classes
- [bpp-new(3)](new.md) for creating new instances of classes
- [bpp-delete(3)](delete.md) for deleting objects
- [bpp-methods(3)](methods.md) for more information on object methods
- [bpp-toprimitive(3)](toprimitive.md) for more information on the `toPrimitive` method
- [bpp-pointers(3)](pointers.md) for more information on pointers to objects
- [bpp-supershell(3)](supershell.md) for more information on supershells
