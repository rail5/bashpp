# Bash++ Compiler Test Suite

This is a test suite for the Bash++ compiler. It is designed to test the compiler's ability to compile and run Bash++ code.

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

@TestStats stats # Initialize an object to store test statistics (pass/fail counts)
@TestRunner runner # Initialize a test runner object
@runner.setStats &@stats # Pass the address of the stats object to the runner, so it can update the stats as tests are run

# Add test cases to the runner
@runner.runTest "Name of the test" "test-suite/tests/path-to-the-test-script.bpp" "Expected output"

# ...

echo "@stats" # Print the test statistics
```
