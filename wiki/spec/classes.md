---
layout: custom
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

The `@class` directive can be used to define a class with or without a parent class. If a parent class is specified, the new class will inherit all of the properties and methods of the parent class. Bash++ only supports **single-inheritance**. This means that a class can only inherit from one parent class. The parent class must be defined before the child class in the code.

The class name must be a valid Bash++ identifier. This means that it can only contain letters, numbers, and underscores, cannot start with a number, cannot be a reserved word, and cannot contain two consecutive underscores. The class name is case-sensitive. The class name must be unique within the current scope. If a class with the same name already exists in the current scope, an error will be generated.

After defining a class, you can create objects of that class using the `@TYPE ID` syntax. For example, if you define a class called `MyClass`, you can create an object of that class using `@MyClass myObject`. You can also create a pointer to an object of that class using the `@TYPE* ID` syntax. For example, `@MyClass* myObjectPtr=@new MyClass`. The pointer will be initialized to point to a new instance of the class.

The class can contain the following:

 - **Data Members**: These are variables that hold the state of the object. They can be of any primitive type, or they can be objects or pointers to objects. Data members can be declared as `@private`, `@protected`, or `@public`.
 - **Methods**: These are functions that operate on the data members of the class. They can be declared as `@private`, `@protected`, or `@public`. Methods can also be declared as `@virtual`, which means that they can be overridden in a derived class. The method name must be a valid Bash++ identifier. The method name is case-sensitive. The method name must be unique within the class. If a method with the same name already exists in the class, an error will be generated.
 - **Constructors**: These are special methods that are called when an object of the class is created. They are used to initialize the object. A class can only have one constructor. You can define a constructor using the `@constructor` keyword.
 - **Destructors**: These are special methods that are called when an object of the class is destroyed. They are used to clean up any resources that the object may have allocated. A class can only have one destructor. You can define a destructor using the `@destructor` keyword.

**A note on constructors and destructors**: The constructor and destructor are called automatically when an object is created or destroyed. However, there are also two **system** methods that bookend the creation/destruction of an object: `__new` and `__delete`. These methods are called before the constructor and after the destructor, respectively. They are used to allocate and deallocate memory for the object. These methods cannot be overridden.

# EXAMPLE

```bash
@class DerivedClass : BaseClass {
	@private dataMember="Default value"
	@private @Object* objectMember=@new Object

	@public @method setDataMember data {
		@this.dataMember="$data"
	}

	@constructor {
		echo "DerivedClass constructor called"
	}
}
@DerivedClass derivedObject
@DerivedClass* derivedPointer=@new DerivedClass

@derivedObject.setDataMember "New value"
@derivedPointer.setDataMember "New value"
```

# NOTES

Classes cannot contain commands directly. All commands must be contained within methods, constructors, or destructors. This is because classes are not executed in a shell context. They are simply blueprints for creating objects. If you place commands directly in a class, you'll get a compiler error along the lines of "code outside of a code entity."

# SEE ALSO

 - [bpp-methods(3)](methods.md) for more information on object methods
 - [bpp-toprimitive(3)](toprimitive.md) for more information on the `toPrimitive` method
 - [bpp-new(3)](new.md) for creating an object
 - [bpp-delete(3)](delete.md) for deleting an object
