#pragma once

#include <cstddef>

namespace ffb {

/// Default buffer policy: all formatting features enabled.
///
/// A policy is a plain struct with `static constexpr bool` feature flags
/// and type aliases that control the argument and internal processing types
/// used by FixedFormatBuffer::Format.
///
/// @par Type aliases
/// - `IntType`   — the type read from the va_list for @c %i / @c %d.
///                 Must be a type that survives default argument promotion
///                 (i.e. not narrower than @c int). Default: @c int.
/// - `FloatType` — the internal floating-point type used by the formatter
///                 for @c %f processing. Default: @c float.
///
/// @par Feature flags
/// - `kSupportFloatingPointDecimals` — enables @c %f formatting. When @c false the specifier
///                     is silently consumed but produces no output.
/// - `kDefaultFloatPrecision` — decimal digits after the point for bare @c %f. Default: 6.
///
/// @par Extending the policy
/// Derive or define a new struct overriding only what you need:
/// @code
///   struct Int64Policy {
///       static constexpr bool kSupportFloatingPointDecimals = true;
///       using IntType   = long long;  // accept 64-bit integers via %i/%d
///       using FloatType = float;
///   };
/// @endcode
struct AllFeatures {
    /// When @c true, the @c %f specifier formats floating-point decimals.
    /// When @c false, @c %f is silently consumed from the argument list
    /// but produces no output, and all float formatting code is compiled away.
    static constexpr bool kSupportFloatingPointDecimals = true;

    /// Number of decimal digits emitted after the point for @c %f when no
    /// explicit precision is given (e.g. @c %f vs @c %.2f).
    /// Must be in the range [0, kMaxFloatPrecision].
    static constexpr std::size_t kDefaultFloatPrecision = 6U;

    /// Type read from the va_list for the @c %i / @c %d specifier.
    /// Must be at least as wide as @c int (no implicit narrowing via va_arg).
    using IntType = int;

    /// Internal floating-point type used for @c %f decomposition.
    /// va_arg always promotes to @c double; the value is cast to this type
    /// before processing.
    using FloatType = float;
};

}  // namespace ffb
