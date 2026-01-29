# Bash++ Compiler Test Suite

This is a test suite for the Bash++ compiler. It is designed to test the compiler's ability to compile and run Bash++ code.

## Latest Results

<img src="https://bpp.sh/badge.svg?nocache" title="" alt="tests-pass-rate" width="137">

<img src="https://bpp.sh/test-results.svg?nocache" title="" alt="test-results" width="300">

## Running the Test Suite

First, compile the Bash++ compiler by running `make` in the root directory of the project.

Then, run the test suite by running `make test` in the root directory of the project.

You can run *specific* test cases by passing an argument to the script. For example, `bin/bpp test-suite/run.bpp "Hello, World" "Supershells"` will run only the test cases named "Hello, World" and "Supershells".

### Options

 - `-h`: Display help
 - `-l`: List all test cases

Calling `run.bpp` with no arguments will run all the test cases.

## Test Suite Structure

The test suite is itself written in Bash++.

 - A test case is a Bash++ script saved in `test-suite/tests/sources/test-name.bpp`
 - The expected output of the test is saved in `test-suite/tests/expected/test-name` (no file extension)
 - The test runner compares the actual output of the test case to the expected output. If they match, the test passes. If they don't match, the test fails.

### Adding New Test Cases

To add a new test case:

1. Create a new Bash++ script in `test-suite/tests/sources/` with a `.bpp` extension.
2. Create a corresponding "expected output" file in `test-suite/tests/expected/` with the same name as the test case (but no `.bpp` extension).

The test runner treats each line of the expected output file as a **regular expression**. This allows for variable output to be matched correctly.

For example, suppose a test is expected to print "hello world" on line 1, and the phrase "Your lucky number is: X" on line 2, where X can be any integer. The expected output file would be written as:

```
hello world
Your lucky number is: [0-9]+
```
