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

Individual test cases are located in the `tests/sources` subdirectory. Each test case is a Bash++ script. The script is executed and its output is compared to the expected output, stored in `tests/expected`.
