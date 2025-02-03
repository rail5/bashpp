# Bash++ Scripts

This directory contains some example scripts that demonstrate how to use Bash++.

More detailed descriptions of the language can be found at the [website](https://bpp.sh/language.html).

 - [toprimitive.bpp](toprimitive.bpp) - Demonstrates how to use **.toPrimitive** methods

 - [constructors-and-destructors.bpp](constructors-and-destructors.bpp) - Demonstrates how to use **constructors** and **destructors**

 - [supershell.bpp](supershell.bpp) - Demonstrates how to use Bash++ **supershells**

 - [include.bpp](include.bpp) - Demonstrates how to use the **include** directive

 - [inheritance.bpp](inheritance.bpp) - Demonstrates **inheritance** and **polymorphism** in Bash++

 - [pointers.bpp](pointers.bpp) - Demonstrates how to use **pointers** in Bash++

 - [queue.bpp](queue.bpp) - A small example of a **queue** implemented in Bash++ using pointers

 - [casting.bpp](casting.bpp) - Demonstrates how to use **type casting** in Bash++

The Bash++ compiler is currently in pre-alpha stage, so some features may not work as expected.

As of now (2025-02-02):
| Example Script                  | Works? | Notes                                                                 |
|---------------------------------|--------|-----------------------------------------------------------------------|
| toprimitive.bpp                 | Yes    | Fully functional                                                      |
| constructors-and-destructors.bpp| Yes    | Fully functional                                                      |
| supershell.bpp                  | Yes    | Fully functional                                                      |
| include.bpp                     | Yes    | Fully functional                                                      |
| inheritance.bpp                 | No     | `@virtual` method overriding has not been implemented                 |
| pointers.bpp                    | No     | A mess                                                                |
| queue.bpp                       | No     | Everything works except the `while` loop at the end -- the supershell is not re-evaluated each time. If you instead put the call to `@testQueue.isEmpty` in a *subshell*, it works perfectly well |
| casting.bpp                     | No     | These casting methods have not been implemented                        |
