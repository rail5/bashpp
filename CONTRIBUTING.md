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
<details>
<summary>See example</summary>
<table>
<tr><th>Bad</th><th>Good</th></tr>
<tr><td>
<pre>char* ptr = dest;
char* str = src;
while (*ptr++ = *str++);</pre>
</td>
<td>
<pre>std::string source;
std::string string_copy = source;</pre>
</td></tr>
</table>
</details>

 - **Write code to be read by humans**
   - Prioritize readability and maintainability
   - Comment non-obvious code thoroughly
   - Use clear, descriptive names for variables, functions, classes, etc
   - *Don't abbreviate!* An excessively long/verbose name is preferable to an unclear one
   - Performance is secondary to correctness and maintainability
<details>
<summary>See example</summary>
<table>
<tr><th>Bad</th><th>Good</th></tr>
<tr><td>
<pre>int x = y + z;
if (x &gt; 10) f();</pre>
</td>
<td>
<pre>int price = cost_price + markup;
if (price &gt; max_price_allowed) apply_discount();</pre>
</td></tr>
</table>
</details>

 - **Process as little Bash as possible**
   - Let Bash do what Bash does, and focus on Bash++-specific functionality

<details>
<summary>See example</summary>
<table>
<tr><th>Bad</th><th>Good</th></tr>
<tr><td>
<pre>auto resolved_binary
  = find_in_path(command->program_name());
if (!resolved_binary) {
  report_error("command not found: "
    + command->program_name()); 
}</pre>
</td>
<td>
<pre>// The user gave a shell command
// We don't need to care what it is or if it works
// We let Bash handle that at runtime.</pre>
</td></tr>
</table>
</details>

 - **When in doubt, do whatever C++ does**
   - If you're not sure how something should behave, look to C++ for guidance
   - Deviate if there's a good reason to do so, but document the reason clearly

 - **Unify, unify, unify!**
   - "Special-casing" should be labelled explicitly as a **hack** and marked with a **TODO** comment
   - Try your best to find a general solution that covers all cases
<details>
<summary>See example</summary>
<table>
<tr><th>Bad</th><th>Good</th></tr>
<tr><td>
<pre>for (const auto& m : methods) {
  // TODO(@developer): HACK.
  // System methods treated specially.
  // This should be unified later.
  if (m->get_name() == "__delete") {
    procedure_1();
  } else if (m->get_name() == "__new") {
    procedure_2();
  } else {
    procedure_3();
  }
}</pre>
</td>
<td>
<pre>for (const auto& m : methods) {
  general_procedure();
}</pre>
</td></tr>
</table>
</details>

## License

By contributing to Bash++, you **confirm** that the work is **your own** and you **agree** that your contributions will be licensed under the same license as the project, which is the GNU General Public License v3.0 (GPL-3.0).

Thank you for helping to make Bash++ better!
