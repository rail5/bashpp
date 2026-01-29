# Contributing to Bash++

Thank you for your interest in contributing to Bash++!

## Reporting Issues

If you encounter any bugs or issues while using Bash++, please report them by opening an issue on our [GitHub Issues page](https://github.com/rail5/bashpp/issues). Please provide as much detail as possible, including steps to reproduce the issue, expected behavior, and any relevant error messages.

## Contributing Code

If you would like to contribute bug fixes, new features, improvements, etc to Bash++, please do so by opening a pull request on our [GitHub repository](https://github.com/rail5/bashpp). Here are some guidelines to follow:

1. Please try to follow the coding style and conventions used in the project.

2. Please write clear commit messages that accurately reflect the changes made.

3. If your changes introduce new functionality, please add the appropriate **regression tests** under `test-suite/` to ensure that the new functionality works as expected and does not break existing functionality.

4. Before submitting a pull request, please ensure that all existing tests pass and that your new tests (if any) also pass. You can run the test suite with `make test`.


### Design Principles

In general, there are some design principles we try to follow in Bash++ development, although these are *aspirational* rather than *enforced*:

 - **K.I.S.S.**
   - Prefer simple, straightforward solutions
   - Don't be clever

 - **Write code to be read by humans**
   - Prioritize readability and maintainability
   - Comment non-obvious code thoroughly
   - Use clear, descriptive names for variables, functions, classes, etc
   - *Don't abbreviate!* An excessively long/verbose name is preferable to an unclear one
   - Performance is secondary to correctness and maintainability

 - **Process as little Bash as possible**
   - Let Bash do what Bash does, and focus on Bash++-specific functionality

 - **When in doubt, do whatever C++ does**
   - If you're not sure how something should behave, look to C++ for guidance
   - Deviate if there's a good reason to do so, but document the reason clearly

 - **Unify, unify, unify!**
   - "Special-casing" should be labelled explicitly as a **hack** and marked with a **TODO** comment
   - Try your best to find a general solution that covers all cases

## License

By contributing to Bash++, you **confirm** that the work is **your own** and you **agree** that your contributions will be licensed under the same license as the project, which is the GNU General Public License v3.0 (GPL-3.0).

Thank you for helping to make Bash++ better!
