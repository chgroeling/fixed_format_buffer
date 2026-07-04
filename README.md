# fixed_format_buffer

Allocation-free fixed-capacity formatting buffer for embedded C++.
No heap, no exceptions, no recursion — suitable for bare-metal and RTOS
environments where `malloc` is unavailable or forbidden. The buffer lives
on the stack, as a class member, or in static memory.

The formatting engine is a complete from-scratch reimplementation: zero
external dependencies, no libc `printf`-family calls.

For formatted output use `CStr()`, which returns a null-terminated `const char*`
that always reflects the current buffer contents.

## Features

- Subset of `snprintf`-style formatting: `%c` `%s` `%d` `%i` `%u` `%x` `%X` `%f`
- Flags: `-` (left-justify), `+` (show sign), ` ` (space), `0` (zero-pad), `#` (alternate form)
- Width and precision (including `*` from argument list)
- Length modifiers: `hh` `h` `l` `ll` `j` `z` `t` `L`
- `Append()` — format and append without overwriting existing content
- `Write()` — copy raw strings into the buffer without format parsing
- `CStr()` — null-terminated `const char*` accessor
- `operator==` / `operator!=` — compare against another buffer or `string_view`
- Compile-time argument whitelist — rejects `std::string` and non-trivial types.
  Integer types wider than the policy (e.g. `long long` with a 32-bit policy)
  are rejected at compile time
- Configurable integer and unsigned types via policy template parameter
- Header-only — single `#include "ffb/fixed_format_buffer.h"`
- C++17

## Motivation

### Why not `snprintf`, `std::string`, or `{fmt}`?

`sprintf`-family functions pull in a large code footprint
and require manual buffer bounds-checking. `std::string` is absent in freestanding C++ and causes heap
fragmentation under high-frequency use. `{fmt}` / `std::format` relies on heap
allocations, exceptions, and substantial compile-time overhead.

`FixedFormatBuffer` is a **single header**, zero heap, no exceptions —
cache-friendly and deterministic.

### Why no output?

This is a pure formatting buffer, not an I/O layer. `CStr()` returns a
null-terminated `const char*` — route it to UART, SPI, BLE, flash, or any
other channel without a dependency on a specific HAL.

### Movable

The move constructor clears the source buffer to empty. Safe in containers
and factory functions.

## Quick start

```cpp
#include "ffb/fixed_format_buffer.h"

ffb::FixedFormatBuffer<64> buf;               // 64-char capacity on the stack

buf.Format("sensor=%d, temp=%.2f C", 7, 23.5f);
buf.CStr();  // → "sensor=7, temp=23.50 C"  (null-terminated)

// Need a string_view? Construct one from CStr() + Size():
std::string_view sv{buf.CStr(), buf.Size()};
// A convenience View() method is deliberately omitted — a stored
// string_view would bake in the buffer's size at construction time
// and silently go stale after the next Format() call, reading into
// overwritten content.  By requiring explicit construction the API
// forces you to think about invalidation at each use site.
//
// If you hold a string_view, re-obtain it after each mutation:
buf.Format("new");
sv = {buf.CStr(), buf.Size()};  // refresh — sv now sees "new"

// Compare against string_view or another buffer:
buf == "hello";             // operator== with string_view
buf == otherBuf;            // operator== with another FixedFormatBuffer

// Write raw strings without format parsing:
buf.Write("hello");         // overwrites with "hello"
buf.Write("world", 3U);     // overwrites with "wor"

// Append formatted content instead of overwriting:
buf.Format("value=");
buf.Append("%d", 42);       // buffer now contains "value=42"

buf.Format("%08x", 0xCAFEU);                 // overwrites the buffer
buf.CStr();  // → "0000cafe"

std::string snapshot{buf.CStr(), buf.Size()};  // copy before overwriting (no strlen)
snapshot;    // → "0000cafe"  (still valid, independent of buffer lifetime)
```

## Custom policy types

The default policy uses `int32_t` / `uint32_t`. A pre-built 64-bit policy
(`ffb::Int64Policy`) is available.  You can also define your own policy
to control the integer width, floating-point type, or to disable float
support entirely (`kSupportFloatingPointDecimals = false`).

```cpp
// Built-in 64-bit policy:
ffb::FixedFormatBuffer<32, ffb::Int64Policy> buf;
buf.Format("%u", UINT64_C(18446744073709551615));
// → "18446744073709551615"

// Custom policy with double-precision floats:
struct DoublePolicy {
    static constexpr bool        kSupportFloatingPointDecimals = true;
    static constexpr std::size_t kDefaultFloatPrecision        = 6U;
    using IntType   = int32_t;
    using UIntType  = uint32_t;
    using FloatType = double;
};

ffb::FixedFormatBuffer<64, DoublePolicy> buf2;
buf2.Format("%.9f", 3.141592653589793);  // NOTE: precision capped at 6
```

> **Note:** Passing types wider than the policy (e.g. `double` to a
> `FloatType=float` policy, or `long long` to the default 32-bit policy)
> is a compile-time error, not silent truncation. Use a matching policy
> (like `ffb::Int64Policy` or a policy with `FloatType = double`) to accept
> wider arguments.

## Building

Requires CMake 3.15+, a C++17 compiler, and GoogleTest.

```sh
conan install . --build=missing
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug
```
