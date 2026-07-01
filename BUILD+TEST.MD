# BUILD+TEST

## Prerequisites

- CMake ≥ 3.15
- Conan 2.x
- A C++17-capable compiler (GCC ≥ 7, Clang ≥ 5, MSVC ≥ 19.14)

## Build & Test

```bash
# 1. Install dependencies via Conan
conan install . --build=missing -s build_type=Debug

# 2. Configure
cmake --preset conan-debug

# 3. Build
cmake --build --preset conan-debug

# 4. Run tests
ctest --preset conan-debug --output-on-failure
```

## Release build

```bash
conan install . --build=missing -s build_type=Release
cmake --preset conan-release
cmake --build --preset conan-release
ctest --preset conan-release --output-on-failure
```
