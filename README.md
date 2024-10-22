# Bash++

Bash with classes

Copright (C) 2024 rail5

## The Basic Idea

A Bash++ script will be read first by the Bash++ interpreter.
The interpreter parses the script, identifies the objects and their methods
At the end, the interpreter will generate an ordinary procedural Bash script, and the generated script will be passed to Bash proper for execution

## Syntax

The Bash++ script will be written in a special syntax
The syntax will be a superset of the Bash syntax
The syntax will be designed to be easily parsed by the Bash++ interpreter
The syntax will be designed to be easily converted to ordinary Bash script
The syntax will be designed to be easily understood by the programmer
The syntax will be designed to be easily written by the programmer

Here is the basic syntax for class definition:

```
@class ClassName {

   @property [public|private|protected] propertyName

   @constructor [parameters] {
	  	constructor body
   }

   @method [public|private|protected] methodName {
	  	method body
   }
}
```

Here is the basic syntax for object creation:

```
@object objectName = new ClassName
```

Here is the basic syntax for method invocation:

```
objectName.methodName
```

Here is the basic syntax for property access:

```
objectName.propertyName
```

Here is the basic syntax for method invocation with parameters:

```
objectName.methodName parameter1 parameter2 parameter3
```

Here is the basic syntax for property assignment:

```
objectName.propertyName = value
```


Here is the basic syntax for property access with assignment:

```
value = objectName.propertyName
```

Here is the basic syntax for method invocation with parameters and assignment:

```
value = objectName.methodName parameter1 parameter2 parameter3
```

With all of this, we can write Bash scripts like the following:

```sh
if [[ objectName.methodName parameter1 parameter2 parameter3 -eq 0 ]]; then
	   echo "The method returned 0"
elif [[ objectName.propertyName == "value" ]]; then
	   echo "The property is set to value"
fi
```