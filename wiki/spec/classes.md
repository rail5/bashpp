---
layout: custom
title: Classes
brief: "@class"
description: "Define a class"
---
# NAME

@class - Define a class

# SYNOPSIS

```bash
@class {CLASS-NAME} [: {PARENT-CLASS-NAME}] {
	{@private | @protected | @public} {PRIMITIVE-VARIABLE-NAME}[={DEFAULT-VALUE}]

	{@private | @protected | @public} @{OBJECT-TYPE} {OBJECT-VARIABLE-NAME}

	{@private | @protected | @public} @{POINTER-TYPE}* {POINTER-NAME}[={DEFAULT-VALUE}]

	[@virtual] {@private | @protected | @public} @method {METHOD-NAME} [{ARGUMENTS}] {
		[COMMANDS]
	}

	@constructor {
		[COMMANDS]
	}

	@destructor {
		[COMMANDS]
	}
}
```

# DESCRIPTION

The `@class` directive is used to define a class. A class is a blueprint for creating objects. It defines the properties and methods that the objects of that class will have.

Classes must be defined in the global scope. They cannot be defined within other classes, methods, etc. In our terms, their immediate parent entity must be the program entity.

The `@class` directive can be used to define a class with or without a parent class. If a parent class is specified, the new class will inherit all of the properties and methods of the parent class. Bash++ only supports **single-inheritance**. This means that a class can only inherit from one parent class. The parent class must be defined before the child class in the code.

The class name must be a valid Bash++ identifier. This means that it can only contain letters, numbers, and underscores, cannot start with a number, cannot be a reserved word, and cannot contain two consecutive underscores. The class name is case-sensitive. The class name must be unique within the current scope. If a class with the same name already exists in the current scope, a compile-time error will be generated.

After defining a class, you can create objects of that class using the `@TYPE ID` syntax. For example, if you define a class called `MyClass`, you can create an object of that class using `@MyClass myObject`. You can also create a pointer to an object of that class using the `@TYPE* ID` syntax. For example, `@MyClass* myObjectPtr=@new MyClass`. The pointer will be initialized to point to a new instance of the class.

The class can contain the following:

 - **Data Members**: These are variables that hold the state of the object. They can be of any primitive type, or they can be objects or pointers to objects. Data members can be declared as `@private`, `@protected`, or `@public`.
 - **Methods**: These are functions that operate on the data members of the class. They can be declared as `@private`, `@protected`, or `@public`. Methods can also be declared as `@virtual`, which means that they can be overridden in a derived class and will be dynamically dispatched. The method name must be a valid Bash++ identifier. The method name is case-sensitive. The method name must be unique within the class. If the method name is not unique, a compile-time error will be generated.
 - **Constructors**: These are special methods that are called when an object of the class is created. They are used to initialize the object. A class can only have one constructor. You can define a constructor using the `@constructor` keyword. A derived class inherits the constructor of its parent class, but can also define its own constructor to perform additional initialization.
 - **Destructors**: These are special methods that are called when an object of the class is destroyed. They are used to clean up any resources that the object may have allocated. A class can only have one destructor. You can define a destructor using the `@destructor` keyword. A derived class inherits the destructor of its parent class, but can also define its own destructor to perform additional cleanup.

**A note on constructors and destructors**: The constructor and destructor are called automatically when an object is created or destroyed. However, there are also two **system** methods that bookend the creation/destruction of an object: `__new` and `__delete`. These methods are called before the constructor and after the destructor, respectively. They are used to allocate and deallocate memory for the object. These methods cannot be overridden.

# EXAMPLE

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-class-example-1.html -%}
</code></pre></div>

# NOTES

Classes are not code entities and therefore cannot contain commands directly. All commands must be contained within methods, constructors, or destructors. If you place commands directly in a class, you'll get a compile-time error along the lines of "code outside of a code entity."

## SYSTEM METHODS

System methods are special methods that are called automatically at specific points in the object's lifecycle. The five system methods are:

 - `__new`: This method is called when an object of the class is created, before the constructor is called. It initializes all of the data members of the class to their default values and sets the object's vPointer to the class's vTable. This method is automatically generated by the compiler and cannot be overridden.
 - `__delete`: This method is called when an object of the class is destroyed, after the destructor is called. It is responsible for deallocating any memory that was allocated for the object and performing any necessary cleanup. This method is automatically generated by the compiler and cannot be overridden.
 - `__copy`: This method is called when a non-primitive copy of an object is made. It uses the compiler's internal map of the class's data layout to duplicate the object correctly. This method is automatically generated by the compiler and cannot be overridden.
 - `__constructor`: This method only exists when defined by the user. It is called after `__new` and is responsible for any additional initialization that the user wants to perform on the object. This method can be overridden in derived classes to perform additional initialization.
 - `__destructor`: This method only exists when defined by the user. It is called before `__delete` and is responsible for any additional cleanup that the user wants to perform on the object. This method can be overridden in derived classes to perform additional cleanup.

All system methods are prefixed with double underscores to avoid naming conflicts with user-defined methods.

In addition to the system methods, there is also a `toPrimitive` method that is automatically generated by the compiler for each class. The `toPrimitive` method can be overridden by the user to define how an object of the class should be converted to a primitive value, but will exist even if the user does not explicitly define it. The `toPrimitive` method is automatically called any time that a non-primitive object is used in a context where a primitive value is expected, such as in string interpolation.

## INITIALIZATION

The `__new` system method is responsible for an object's initialization.

Data members are initialized in precisely the order they are declared in the class.

No data members are ever left uninitialized by `__new`. If a data member is not given a default value in the class definition, it will be default-initialized to an empty string. Non-primitive data members (objects) are initialized by calling their `__new` method, and their constructor (if present). Pointers are default-initialized to `@nullptr` if no default value is provided.

### CONSTRUCTORS AND DESTRUCTORS

An object inherits the constructor and destructor of its parent class. If the child class defines its own constructor, it will call the parent class's constructor *first* before executing its own constructor body. If the child class defines its own destructor, it will execute its destructor body *first* before calling the parent class's destructor.

# SEE ALSO

 - [bpp-methods(3)](methods.md) for more information on object methods
 - [bpp-toprimitive(3)](toprimitive.md) for more information on the `toPrimitive` method
 - [bpp-new(3)](new.md) for creating an object
 - [bpp-delete(3)](delete.md) for deleting an object
 - [bpp-entities(3)](entities.md) for more information on entities and code entities
