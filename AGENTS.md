# AGENTS.md - fixed_format_buffer

## Project overview

Allocation-free fixed-capacity formatting buffer for embedded C++. Usable on the stack, as 
a class member, or statically allocated. Formatted string views are transient and valid only 
until the buffer is modified, reused, or destroyed.

## Project structure

```text
<repo-root>/
├── include/                    # Public SDK headers
│   └── ffb/
├── src/                        # Implementation code
│   └── ffb/
├── tests/                      # GoogleTest suites (required)
└── CMakeLists.txt
```

## Naming Conventions (project-level)

The include directory and C++ namespace are both `ffb` (short for `fixed_format_buffer`).
Usage: `#include "ffb/fixed_format_buffer.h"` and `ffb::FixedFormatBuffer<N>`.

## Key Technologies

- **C++17** - primary implementation language
- **CMake** - build system
- **Conan 2** - package manager
- **GoogleTest** - test framework

## Coding Style

### Naming Conventions

| Element           | Convention       | Example                     |
| ----------------- | ---------------- | --------------------------- |
| Classes/structs   | PascalCase       | `FooBarClass`               |
| Public methods    | PascalCase       | `Connect()`, `Initialize()` |
| Local vars/params | camelCase        | `fooId`, `barFactory`       |
| Constants/macros  | UPPER_SNAKE_CASE | `MAX_FOOBAR_SIZE`           |
| Files             | snake_case       | `foobar_class.cpp`          |

## Git Workflow

### Commit Message Format

```text
<type>: <short description>
```

Types:

- `feat`
- `fix`
- `docs`
- `test`
- `refactor`
- `chore` 

