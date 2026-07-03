# fixed_format_buffer

Allocation-free fixed-capacity formatting buffer for embedded C++.

No heap, no exceptions, no recursion — suitable for bare-metal and RTOS
environments where `malloc` is unavailable or forbidden. The buffer lives
on the stack, as a class member, or in static memory.

Formatted string views are transient and valid only until the buffer is
modified, reused, or destroyed.

## Motivation

### Why not `snprintf`, `std::string`, or `{fmt}`?

`sprintf`-family functions pull in a large code footprint
and require manual buffer bounds-checking. `std::string` is absent in freestanding C++ and causes heap
fragmentation under high-frequency use. `{fmt}` / `std::format` relies on heap
allocations, exceptions, and substantial compile-time overhead.

`FixedFormatBuffer` is a **single header**, zero heap, no exceptions —
cache-friendly and deterministic.

### Why no output?

This is a pure formatting buffer, not an I/O layer. `View()` returns a
null-terminated `string_view` — route it to UART, SPI, BLE, flash, or
any other channel without a dependency on a specific HAL.

### Movable

The move constructor clears the source buffer to empty, preventing dangling
`string_view` references. Safe in containers and factory functions.

## Features

- Subset of `printf`-style formatting: `%c` `%s` `%d` `%i` `%u` `%x` `%X` `%f`
- Flags: `-` (left-justify), `+` (show sign), ` ` (space), `0` (zero-pad), `#` (alternate form)
- Width and precision (including `*` from argument list)
- Length modifiers: `hh` `h` `l` `ll` `j` `z` `t` `L`
- Compile-time type checking — rejects `std::string` and other non-trivial types
- Configurable integer and unsigned types via policy template parameter
- Header-only — single `#include "ffb/fixed_format_buffer.h"`
- C++17

## Quick start

```cpp
#include "ffb/fixed_format_buffer.h"

ffb::FixedFormatBuffer<64> buf;               // 64-char capacity on the stack

buf.Format("sensor=%d, temp=%.2f C", 7, 23.5f);
buf.View();  // → "sensor=7, temp=23.50 C"

// View() returns a string_view into the buffer. It is invalidated
// by the next Format() call, Clear(), or when the buffer goes out of scope.
// For a null-terminated C string, use data() on the string_view:

puts(buf.View().data());                      // null-terminated, no copy

std::string snapshot{buf.View()};             // copy before overwriting

buf.Format("%08x", 0xCAFEU);                 // overwrites the buffer
buf.View();  // → "0000cafe"
snapshot;    // → "sensor=7, temp=23.50 C"  (still valid)
```

## Custom policy types

The default policy uses `int32_t` / `uint32_t`. For 64-bit values,
define a custom policy:

```cpp
struct Int64Policy {
    static constexpr bool        kSupportFloatingPointDecimals = true;
    static constexpr std::size_t kDefaultFloatPrecision        = 6U;
    using IntType   = int64_t;
    using UIntType  = uint64_t;
    using FloatType = double;
};

ffb::FixedFormatBuffer<32, Int64Policy> buf;
buf.Format("%u", UINT64_C(18446744073709551615));
// → "18446744073709551615"
```

## Building

Requires CMake 3.15+, a C++17 compiler, and GoogleTest.

```sh
conan install . --build=missing
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug
```
