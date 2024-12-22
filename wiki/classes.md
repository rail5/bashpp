# Bash++: Classes

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

## Data Members

Data members can be declared using the `@private` or `@public` keywords. If a data member is declared as `@public`, it can be accessed from outside the class. If it is declared as `@private`, it can only be accessed from within the class.

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

## Methods

Methods are the functions associated with the class; they're declared using the `@method` keyword. Methods can access both public and private data members of the class. A method can be declared as `@public` or `@private`, just like data members.

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

## Constructors and Destructors

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

You can also define constructors that take arguments:

```bash
@class MyClass {
	@constructor {
		echo "Constructor called"
	}

	@constructor arg1 {
		echo "Constructor called with arguments"
		echo "Argument 1: $arg1"
	}

	@constructor arg1 arg2 {
		echo "Constructor called with arguments"
		echo "Argument 1: $arg1"
		echo "Argument 2: $arg2"
	}

	@constructor @Object obj {
		echo "Constructor called with object"
		echo "Object data member: @obj.dataMember"
	}
}
```

Constructors can be overloaded, meaning that you can define multiple constructors with different numbers or types of arguments.

An object's destructor will be called if:

 - The object goes out of scope

 - The script exits

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

## "toPrimitive" Object Casting

In Bash++, every class has a method called `toPrimitive` that returns a string representation of the object. This method is called automatically when an object is used in a context where a string is expected.

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
		echo "MyClass instance with data member: @dataMember"
	}
}

@MyClass myObject

echo "Object: @myObject" # Calls myObject.toPrimitive
```

In this case, the script will output: `Object: MyClass instance with data member: Hello, world!`.

## Inheritance

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
