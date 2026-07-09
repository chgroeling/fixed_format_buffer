# Contributing to fixed_format_buffer

## Getting started

```sh
git clone https://github.com/chgroeling/fixed_format_buffer.git
cd fixed_format_buffer

# Debug build
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug

# Release build
conan install . --build=missing -s build_type=Release
cmake --preset conan-release
cmake --build --preset conan-release
ctest --preset conan-release
```

## Project layout

```
├── include/ffb/   # Public header (header-only library)
├── tests/         # GoogleTest suites
├── test_package/  # Conan integration test
├── conanfile.py   # Conan 2 recipe
└── CMakeLists.txt
```

## Coding style

| Element           | Convention       |
| ----------------- | ---------------- |
| Classes/structs   | PascalCase       |
| Public methods    | PascalCase       |
| Local vars/params | camelCase        |
| Constants/macros  | UPPER_SNAKE_CASE |
| Files             | snake_case       |

Use **list-initialization** (`{}`) for all variable declarations — never `=`:

```cpp
std::size_t len{0U};
const char first{*fmt++};
```

## Commit messages

```
<type>: <short description>
```

| Type     | Purpose              |
| -------- | -------------------- |
| `feat`   | New feature          |
| `fix`    | Bug fix              |
| `docs`   | Documentation        |
| `test`   | Test changes only    |
| `refactor` | Code restructuring |
| `chore`  | Maintenance / CI     |

## Tests

All changes must pass the existing test suite. New features should include
tests covering normal operation, edge cases, and error conditions.

Test files live under `tests/`. Add new `.cpp` files to
`tests/CMakeLists.txt`.

Run with:

```sh
ctest --preset conan-debug
```
