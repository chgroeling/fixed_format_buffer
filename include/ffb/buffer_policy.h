#pragma once

namespace ffb {

/// Default buffer policy: all formatting features enabled.
///
/// A policy is a plain struct with `static constexpr bool` feature flags.
/// Pass a custom policy as the second template argument of FixedFormatBuffer
/// to disable specific features at compile time.
///
/// @par Extending the policy
/// Add new flags (with `= true` defaults) here and mirror them in custom
/// policy structs as needed:
/// @code
///   struct NoFloat {
///       static constexpr bool kFloatSupport = false;
///   };
/// @endcode
struct AllFeatures {
    /// Enable formatting of floating-point arguments (float, double, long double).
    static constexpr bool kFloatSupport = true;
};

}  // namespace ffb
