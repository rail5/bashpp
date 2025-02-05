# Bash++ Compiler Test Suite

This is a test suite for the Bash++ compiler. It is designed to test the compiler's ability to compile and run Bash++ code.

## Latest Results

<img src="https://vp.rail5.org/bpp/badge.svg?nocache" title="" alt="tests-pass-rate" width="137">

<img src="https://vp.rail5.org/bpp/test-results.svg?nocache" title="" alt="test-results" width="360">

## Running the Test Suite

First, compile the Bash++ compiler by running `make` in the root directory of the project.

Then, run the test suite by running `make test` in the root directory of the project.

## Test Suite Structure

The test suite is itself written in Bash++.

Individual test cases are located in the `tests` subdirectory. Each test case is a Bash++ script. The test suite runs each test case and checks the output against the expected output.

Tests are configured in `run.bpp` in the following format:

```bash
# Necessary includes
@include_once "detect-tty.bpp"
@include_once "TestStats.bpp"
@include_once "Test.bpp"
@include_once "TestRunner.bpp"

@TestRunner runner # Initialize a test runner object

# Add test cases to the runner
@runner.runTest "Name of the test" "test-suite/tests/path-to-the-test-script.bpp" "Expected output"

# ...

echo "@runner.stats" # Print the test statistics
```
