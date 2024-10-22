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
