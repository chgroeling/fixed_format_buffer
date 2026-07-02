#pragma once

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
/// - `kFloatSupport` — enables @c %f formatting. When @c false the specifier
///                     is silently consumed but produces no output.
///
/// @par Extending the policy
/// Derive or define a new struct overriding only what you need:
/// @code
///   struct Int64Policy {
///       static constexpr bool kFloatSupport = true;
///       using IntType   = long long;  // accept 64-bit integers via %i/%d
///       using FloatType = float;
///   };
/// @endcode
struct AllFeatures {
    /// Enable formatting of floating-point arguments.
    static constexpr bool kFloatSupport = true;

    /// Type read from the va_list for the @c %i / @c %d specifier.
    /// Must be at least as wide as @c int (no implicit narrowing via va_arg).
    using IntType = int;

    /// Internal floating-point type used for @c %f decomposition.
    /// va_arg always promotes to @c double; the value is cast to this type
    /// before processing.
    using FloatType = float;
};

}  // namespace ffb
