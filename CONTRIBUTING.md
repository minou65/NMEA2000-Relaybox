# Contributing

## Commit Messages
- Commit messages must always be written in English.

## Code Comments
- Code comments must always be written in English.

## C++ Variable Names
- Module-level variables and constants have neither a leading nor a trailing underscore: `variable`.
- Function/method-local variables, including loop variables and lambda captures of locals, have a trailing underscore: `variable_`.
- Function/method parameters, including lambda parameters, have neither a leading nor a trailing underscore.
- Class and struct member names are unchanged by this convention.
- Apply these rules to C++ source, headers, and tests. Python build scripts, macros, and external library symbols are not renamed.