#pragma once

#include <array>
#include <cfloat>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <cstring>
#include <type_traits>

/// @name Library version
/// Canonical version of the library. As this is a header-only library that
/// may be copied directly into a project, the header itself is the single
/// source of truth; the build system (CMake, Conan) derives its version from
/// these macros.
/// @{
#define FFB_VERSION_MAJOR 0
#define FFB_VERSION_MINOR 3
#define FFB_VERSION_PATCH 1
#define FFB_VERSION_STRING "0.3.1"
/// @}

namespace ffb {

/// Major version component, matching #FFB_VERSION_MAJOR.
constexpr int kVersionMajor{FFB_VERSION_MAJOR};
/// Minor version component, matching #FFB_VERSION_MINOR.
constexpr int kVersionMinor{FFB_VERSION_MINOR};
/// Patch version component, matching #FFB_VERSION_PATCH.
constexpr int kVersionPatch{FFB_VERSION_PATCH};
/// Full version string, e.g. "0.1.1", matching #FFB_VERSION_STRING.
constexpr const char* kVersion{FFB_VERSION_STRING};

/// Default buffer policy: all formatting features enabled.
///
/// A policy is a plain struct with `static constexpr bool` feature flags
/// and type aliases that control the argument and internal processing types
/// used by FixedFormatBuffer::Format.
///
/// @par Type aliases
/// - `IntType`   — the type read from the va_list for @c %i / @c %d.
///                 Must be a type that survives default argument promotion
///                 (i.e. not narrower than @c int). Default: @c int32_t.
/// - `UIntType`  — the type read from the va_list for @c %x / @c %u.
///                 Must be an unsigned type surviving default argument
///                 promotion. Default: @c uint32_t.
/// - `FloatType` — the internal floating-point type used by the formatter
///                 for @c %f processing. Default: @c float.
///
/// @par Feature flags
/// - `kSupportFloatingPointDecimals` — enables @c %f formatting. When @c false
/// the specifier
///                     is silently consumed but produces no output.
/// - `kDefaultFloatPrecision` — decimal digits after the point for bare @c %f.
/// Default: 6.
///
/// @par Extending the policy
/// Derive or define a new struct overriding only what you need:
/// @code
///   struct HighPrecisionPolicy {
///       static constexpr bool kSupportFloatingPointDecimals = true;
///       using IntType   = long long;          // accept 64-bit integers via %i/%d
///       using UIntType  = unsigned long long; // accept 64-bit unsigned via %x/%u
///       using FloatType = double;             // double-precision float type
///   };
/// @endcode
struct StandardPolicy {
    /// When @c true, the @c %f specifier formats floating-point decimals.
    /// When @c false, @c %f is silently consumed from the argument list
    /// but produces no output, and all float formatting code is compiled away.
    static constexpr bool kSupportFloatingPointDecimals = true;

    /// Number of decimal digits emitted after the point for @c %f when no
    /// explicit precision is given (e.g. @c %f vs @c %.2f).
    /// Must be in the range [0, kMaxFloatPrecision].
    static constexpr std::size_t kDefaultFloatPrecision = 6U;

    /// Maximum decimal digits after the point for @c %f.
    /// Explicit precision values above this limit are clamped.
    /// The @c Pow10 lookup table is computed at compile time for this
    /// range, so increasing this value carries a small code-size cost.
    static constexpr std::size_t kMaxFloatPrecision = 6U;

    /// Type read from the va_list for the @c %i / @c %d specifier.
    /// Must be at least as wide as @c int (no implicit narrowing via va_arg).
    using IntType = int32_t;

    /// Type read from the va_list for the @c %x / @c %u specifiers.
    /// Must be an unsigned type surviving default argument promotion.
    using UIntType = uint32_t;

    /// Internal floating-point type used for @c %f decomposition.
    /// va_arg always promotes to @c double; the value is cast to this type
    /// before processing.
    using FloatType = float;
};

/// Policy with 64-bit integer support and double-precision floating-point.
///
/// Use when formatting @c long, @c long long, @c size_t, @c ptrdiff_t,
/// @c intmax_t, or @c uintmax_t arguments, or when @c double precision
/// is needed for @c %f formatting.
struct HighPrecisionPolicy {
    /// @copydoc StandardPolicy::kSupportFloatingPointDecimals
    static constexpr bool kSupportFloatingPointDecimals = true;

    /// @copydoc StandardPolicy::kDefaultFloatPrecision
    static constexpr std::size_t kDefaultFloatPrecision = 6U;

    /// @copydoc StandardPolicy::kMaxFloatPrecision
    static constexpr std::size_t kMaxFloatPrecision = 6U;

    /// 64-bit signed integer type for @c %i / @c %d.
    using IntType = int64_t;

    /// 64-bit unsigned integer type for @c %x / @c %u.
    using UIntType = uint64_t;

    /// Internal floating-point type for @c %f.
    using FloatType = double;
};

/// Policy with floating-point formatting disabled.
///
/// The @c %f specifier is still parsed and its argument consumed (so the
/// variadic argument list stays aligned), but it produces no output and all
/// float-formatting code is compiled away.  Use this on targets without an
/// FPU or where float output is undesirable to save code space.
struct NoFloatPolicy {
    /// Disables @c %f output; the argument is consumed but nothing is emitted.
    static constexpr bool kSupportFloatingPointDecimals = false;

    /// @copydoc StandardPolicy::kDefaultFloatPrecision
    static constexpr std::size_t kDefaultFloatPrecision = 6U;

    /// @copydoc StandardPolicy::kMaxFloatPrecision
    static constexpr std::size_t kMaxFloatPrecision = 6U;

    /// Type read from the va_list for the @c %i / @c %d specifier.
    using IntType = int32_t;

    /// Type read from the va_list for the @c %x / @c %u specifiers.
    using UIntType = uint32_t;

    /// Internal floating-point type (unused while float output is disabled).
    using FloatType = float;
};

/// Allocation-free fixed-capacity formatting buffer.
///
/// Usable on the stack, as a class member, or statically allocated.
/// Use CStr() to obtain a null-terminated const char* that always
/// reflects the current buffer contents.
///
/// @tparam N      Maximum number of characters (excluding null terminator).
/// @tparam Policy Feature-flag policy struct.
///                Defaults to StandardPolicy (all features enabled).
///
/// @warning Passing an integer type wider than the policy's IntType or
///          UIntType (e.g. a 64-bit integer to a 32-bit policy) is rejected
///          at compile time — such a mismatch would cause undefined behaviour
///          in the variadic argument list.  Use a custom policy with matching
///          type aliases (e.g. IntType = int64_t) to handle wider values.
template <std::size_t N, typename Policy = StandardPolicy>
class FixedFormatBuffer {
public:
    /// Maximum number of characters the buffer can hold (excluding null
    /// terminator).
    enum : std::size_t { CAPACITY = N };

    FixedFormatBuffer() noexcept { buffer_[0] = '\0'; }

    /// Copy constructor (same N and Policy).
    FixedFormatBuffer(const FixedFormatBuffer& other) noexcept
        : buffer_{other.buffer_}, size_{other.size_} {}

    /// Copy assignment (same N and Policy).
    FixedFormatBuffer& operator=(const FixedFormatBuffer& other) noexcept {
        if (this != &other) {
            buffer_ = other.buffer_;
            size_ = other.size_;
        }
        return *this;
    }

    /// Move constructor (same N and Policy).
    FixedFormatBuffer(FixedFormatBuffer&& other) noexcept
        : buffer_{std::move(other.buffer_)}, size_{other.size_} {
        other.size_ = 0;
        other.buffer_[0] = '\0';
    }

    /// Move assignment (same N and Policy).
    FixedFormatBuffer& operator=(FixedFormatBuffer&& other) noexcept {
        if (this != &other) {
            buffer_ = std::move(other.buffer_);
            size_ = other.size_;
            other.size_ = 0;
            other.buffer_[0] = '\0';
        }
        return *this;
    }

    ~FixedFormatBuffer() = default;

    /// Compile-time argument type whitelist.
    ///
    /// Integer arguments wider than the policy types (which would cause
    /// undefined behaviour when read by va_arg via the default no-modifier
    /// path) are rejected at compile time.  Use length modifiers (%ld,
    /// %lld, …) with a policy whose IntType/UIntType is at least as wide.
    static constexpr std::size_t kMaxSafeArgSize =
        (sizeof(typename Policy::IntType) > sizeof(typename Policy::UIntType))
            ? sizeof(typename Policy::IntType)
            : sizeof(typename Policy::UIntType);

    /// Maximum safe width for floating-point arguments.
    /// Rejects @c double when the policy uses @c float, etc.
    static constexpr std::size_t kMaxSafeFloatSize = sizeof(typename Policy::FloatType);

    template <typename T>
    static constexpr bool kIsValidFormatArg =
        std::is_same<T, char>::value || std::is_same<T, signed char>::value ||
        std::is_same<T, unsigned char>::value || std::is_same<T, short>::value ||
        std::is_same<T, unsigned short>::value || std::is_same<T, int>::value ||
        std::is_same<T, unsigned int>::value || std::is_same<T, int16_t>::value ||
        std::is_same<T, uint16_t>::value || std::is_same<T, int32_t>::value || std::is_same<T, uint32_t>::value ||
        (std::is_same<T, long>::value && sizeof(long) <= kMaxSafeArgSize) ||
        (std::is_same<T, unsigned long>::value && sizeof(unsigned long) <= kMaxSafeArgSize) ||
        (std::is_same<T, long long>::value && sizeof(long long) <= kMaxSafeArgSize) ||
        (std::is_same<T, unsigned long long>::value && sizeof(unsigned long long) <= kMaxSafeArgSize) ||
        (std::is_same<T, int64_t>::value && sizeof(int64_t) <= kMaxSafeArgSize) ||
        (std::is_same<T, uint64_t>::value && sizeof(uint64_t) <= kMaxSafeArgSize) ||
        std::is_same<T, float>::value ||
        (std::is_same<T, double>::value && sizeof(double) <= kMaxSafeFloatSize) ||
        (std::is_same<T, long double>::value && sizeof(long double) <= kMaxSafeFloatSize) ||
        std::is_same<T, const char*>::value || std::is_same<T, char*>::value ||
        std::is_same<T, std::nullptr_t>::value;

    /// C++14 emulation of (kIsValidFormatArg<Args> && ...) fold expression.
    template <typename... Args>
    static constexpr bool CheckAllValid() {
        const bool results[] = {kIsValidFormatArg<Args>...};
        for (std::size_t i{0U}; i < sizeof...(Args); ++i) {
            if (!results[i])
                return false;
        }
        return true;
    }

    /// Variadic format. All arguments must be integral, floating-point,
    /// @c const @c char*, @c char*, or @c std::nullptr_t — passing
    /// @c std::string or other non-trivial types is rejected at compile
    /// time. Integer types wider than the policy's @c IntType / @c UIntType
    /// are also rejected.
    ///
    /// @note This check only validates that argument <em>kinds</em> are
    ///       compatible with the variadic argument list. It does @em not
    ///       verify that each argument matches its corresponding format
    ///       specifier — passing a @c float to @c %d or an @c int to
    ///       @c %s is still undefined behaviour. The caller is responsible
    ///       for ensuring that argument types and format specifiers match.
    ///
    /// Supported syntax: @c %[flags][width][.precision][length]specifier
    ///
    /// Supported specifiers:
    ///   - @c %c  — single character (@c char)
    ///   - @c %s  — null-terminated string (@c const @c char*)
    ///   - @c %d, @c %i  — signed decimal integer
    ///   - @c %u  — unsigned decimal integer
    ///   - @c %x  — hexadecimal unsigned integer (lowercase)
    ///   - @c %X  — hexadecimal unsigned integer (uppercase)
    ///   - @c %f  — decimal float; only when @c
    ///   Policy::kSupportFloatingPointDecimals is true.
    ///             Optional precision: @c %.Nf  (default: @c
    ///             Policy::kDefaultFloatPrecision).
    ///
    /// @note The @c - flag (left-align), @c + flag (always show sign),
    ///       @c   (space) flag (prefix non-negative numbers with a space),
    ///       @c 0 flag (zero-pad numeric values) and @c # flag (alternate form)
    ///       are supported.
    ///
    ///       Length modifiers (@c h @c hh @c l @c ll @c j @c z @c t @c L)
    ///       control the type read from the argument list via @c va_arg.
    ///       The type is always read at the width indicated by the modifier
    ///       to keep the variadic argument list aligned, and the resulting value
    ///       is then cast to the policy's @c IntType / @c UIntType / @c FloatType
    ///       for formatting.  This means the policy types determine the output
    ///       range (values wider than the policy type are truncated), but the
    ///       @c va_arg read itself is always safe regardless of policy width.
    ///
    ///       Width produces space-padded output aligned according to the @c -
    ///       flag, or zero-padded for numeric specifiers when @c 0 is active.
    ///       Width and precision support the @c * notation (value read from the
    ///       argument list).  A negative @c * width implies left-justification;
    ///       a negative @c * precision is treated as if the precision were
    ///       omitted.
    ///
    /// Truncates silently if the result exceeds capacity.
    /// @return Number of characters written (excluding null terminator).
    template <typename... Args>
    std::size_t Format(const char* fmt, Args... args) noexcept {
        static_assert(CheckAllValid<Args...>(),
                      "Format argument type not supported. "
                      "Use integer, floating-point, or const char* types.");
        return FormatVa(fmt, args...);
    }

    /// Clear the buffer contents.
    void Clear() noexcept {
        buffer_[0] = '\0';
        size_ = 0;
    }

    /// Return the number of characters currently stored.
    std::size_t Size() const noexcept { return size_; }

    /// Return true if the buffer is empty.
    bool Empty() const noexcept { return size_ == 0; }

    /// Return a null-terminated C-string over the current contents.
    /// Valid only until the buffer is modified or destroyed.
    const char* CStr() const noexcept { return buffer_.data(); }

    /// Write a raw null-terminated string (no formatting) to the buffer,
    /// overwriting existing content. Silently truncates at capacity.
    /// @return Number of characters written (excluding null terminator).
    std::size_t Write(const char* s) noexcept {
        const char* p{s};
        std::size_t len{0U};
        if (!p)
            return len;
        while (*p && len < N) {
            buffer_[len++] = *p++;
        }
        buffer_[len] = '\0';
        size_ = len;
        return size_;
    }

    /// Write a raw string of known length (no formatting) to the buffer,
    /// overwriting existing content. Silently truncates at capacity.
    /// @return Number of characters written (excluding null terminator).
    std::size_t Write(const char* s, std::size_t len) noexcept {
        std::size_t actual{0U};
        if (!s || len == 0U) {
            buffer_[0] = '\0';
            size_ = 0U;
            return size_;
        }
        while (actual < len && actual < N) {
            buffer_[actual] = s[actual];
            ++actual;
        }
        buffer_[actual] = '\0';
        size_ = actual;
        return size_;
    }

    /// Append format. Like Format() but appends to existing
    /// content instead of overwriting.
    /// @see Format()
    template <typename... Args>
    std::size_t Append(const char* fmt, Args... args) noexcept {
        static_assert(CheckAllValid<Args...>(),
                      "Format argument type not supported. "
                      "Use integer, floating-point, or const char* types.");
        return AppendVa(fmt, args...);
    }

    friend bool operator==(const FixedFormatBuffer& a, const FixedFormatBuffer& b) noexcept {
        return std::strcmp(a.CStr(), b.CStr()) == 0;
    }
    friend bool operator!=(const FixedFormatBuffer& a, const FixedFormatBuffer& b) noexcept {
        return !(a == b);
    }
    friend bool operator==(const FixedFormatBuffer& buf, const char* sv) noexcept {
        return sv && std::strcmp(buf.CStr(), sv) == 0;
    }
    friend bool operator==(const char* sv, const FixedFormatBuffer& buf) noexcept {
        return sv && std::strcmp(sv, buf.CStr()) == 0;
    }
    friend bool operator!=(const FixedFormatBuffer& buf, const char* sv) noexcept {
        return !sv || std::strcmp(buf.CStr(), sv) != 0;
    }
    friend bool operator!=(const char* sv, const FixedFormatBuffer& buf) noexcept {
        return !sv || std::strcmp(sv, buf.CStr()) != 0;
    }

private:
    /// C-variadic bridge — called by the template Format().
    std::size_t FormatVa(const char* fmt, ...) noexcept {
        va_list args;
        va_start(args, fmt);
        size_ = DoFormat(fmt, args, 0U);
        va_end(args);
        return size_;
    }

    /// C-variadic bridge — called by the template Append().
    std::size_t AppendVa(const char* fmt, ...) noexcept {
        va_list args;
        va_start(args, fmt);
        size_ = DoFormat(fmt, args, size_);
        va_end(args);
        return size_;
    }

    // -------------------------------------------------------------------------
    // Policy-derived type aliases (used throughout the private implementation)
    // -------------------------------------------------------------------------
    using IntType = typename Policy::IntType;
    using UIntType = typename Policy::UIntType;
    using FloatType = typename Policy::FloatType;

    // -------------------------------------------------------------------------
    // Format flags (parsed from the specifier, e.g. "%-+10.2f")
    // -------------------------------------------------------------------------
    struct FormatFlags {
        bool left_justify : 1;    ///< @c - flag: pad on the right (left-justify
                                  ///< content)
        bool show_sign : 1;       ///< @c + flag: always emit a sign for numeric values
        bool space_flag : 1;      ///< @c (space) flag: prepend space for non-negative
                                  ///< numbers
        bool zero_pad : 1;        ///< @c 0 flag: zero-pad numeric values to width
        bool alternate_form : 1;  ///< @c # flag: prepend 0x for @c \%x, force point
                                  ///< for @c \%f
    };

    /// Parsed length modifier (e.g. @c hh, @c l, @c ll, @c j, @c z, @c t, @c L).
    enum class LengthMod : uint8_t {
        None,
        hh,  ///< @c hh — char / unsigned char
        h,   ///< @c h  — short / unsigned short
        l,   ///< @c l  — long / unsigned long
        ll,  ///< @c ll — long long / unsigned long long
        j,   ///< @c j  — intmax_t / uintmax_t
        z,   ///< @c z  — size_t
        t,   ///< @c t  — ptrdiff_t
        L,   ///< @c L  — long double
    };

    /// Fully parsed conversion specification (everything between @c % and the
    /// specifier letter). Populated by the Parse* helpers before dispatch.
    struct FormatSpec {
        FormatFlags flags{};
        std::size_t width{0U};
        std::size_t precision{Policy::kDefaultFloatPrecision};
        LengthMod len_mod{LengthMod::None};
    };

    // -------------------------------------------------------------------------
    // Output gadget
    //
    // Wraps the destination buffer. pos always increments even past max_chars,
    // giving snprintf-style "would have written" semantics for free.
    // -------------------------------------------------------------------------
    struct Gadget {
        char* buf;
        std::size_t pos;
        std::size_t max_chars;

        void Put(char c) noexcept {
            if (pos < max_chars)
                buf[pos] = c;
            ++pos;
        }
    };

    // -------------------------------------------------------------------------
    // Low-level writers
    // -------------------------------------------------------------------------

    /// Emit `width - content_len` space characters (right-align padding).
    /// No-op when content already meets or exceeds the width.
    static void EmitPadding(Gadget& g,
                            std::size_t width,
                            std::size_t content_len,
                            char pad = ' ') noexcept {
        while (content_len < width) {
            g.Put(pad);
            ++content_len;
        }
    }

    /// A gadget that counts characters without writing them.
    /// Used to measure content length before emitting padding.
    static Gadget MakeCountingGadget() noexcept {
        // max_chars == 0 ensures Put() never dereferences buf.
        return Gadget{nullptr, 0U, 0U};
    }

    /// Emit @p write_content with space padding to @p width, honouring the
    /// left/right justification. The writer is invoked once against a counting
    /// gadget to measure its length, then again against the real gadget.
    template <typename WriteContent>
    static void EmitSpacePadded(Gadget& g,
                                std::size_t width,
                                bool left_justify,
                                WriteContent write_content) noexcept {
        Gadget dry{MakeCountingGadget()};
        write_content(dry);
        if (!left_justify)
            EmitPadding(g, width, dry.pos);
        write_content(g);
        if (left_justify)
            EmitPadding(g, width, dry.pos);
    }

    /// Emit a right-justified, zero-padded numeric value. The sign/prefix
    /// (@p write_prefix) is emitted before the zero fill; the magnitude
    /// (@p write_magnitude) follows. Padding is measured against the combined
    /// prefix + magnitude length.
    template <typename WritePrefix, typename WriteMagnitude>
    static void EmitZeroPadded(Gadget& g,
                               std::size_t width,
                               WritePrefix write_prefix,
                               WriteMagnitude write_magnitude) noexcept {
        Gadget dry{MakeCountingGadget()};
        write_prefix(dry);
        write_magnitude(dry);
        write_prefix(g);
        EmitPadding(g, width, dry.pos, '0');
        write_magnitude(g);
    }

    static void WriteRaw(Gadget& g, const char* s, std::size_t len) noexcept {
        for (std::size_t i = 0U; i < len; ++i)
            g.Put(s[i]);
    }

    static void WriteString(Gadget& g, const char* s) noexcept {
        if (!s) {
            WriteRaw(g, "(null)", 6U);
            return;
        }
        while (*s)
            g.Put(*s++);
    }

    /// Write an unsigned 64-bit integer in decimal.
    static void WriteUnsigned(Gadget& g, uint64_t value) noexcept {
        char tmp[20]{};  // 2^64 needs at most 20 decimal digits
        std::size_t len{0U};
        if (value == 0ULL) {
            g.Put('0');
            return;
        }
        while (value) {
            tmp[len++] = static_cast<char>('0' + value % 10U);
            value /= 10U;
        }
        while (len)
            g.Put(tmp[--len]);  // reverse
    }

    static uint64_t AbsAsU64(IntType value) noexcept {
        if (value < IntType(0))
            return static_cast<uint64_t>(-(static_cast<int64_t>(value) + 1)) + 1ULL;
        return static_cast<uint64_t>(value);
    }

    static void WriteHex(Gadget& g, UIntType value, bool uppercase = false) noexcept {
        static constexpr char kDigitsLower[] = "0123456789abcdef";
        static constexpr char kDigitsUpper[] = "0123456789ABCDEF";
        const char* const kDigits{uppercase ? kDigitsUpper : kDigitsLower};
        char tmp[sizeof(UIntType) * 2]{};
        std::size_t len{0U};
        if (value == UIntType(0)) {
            g.Put('0');
            return;
        }
        while (value) {
            tmp[len++] = kDigits[value & UIntType(0x0F)];
            value >>= UIntType(4);
        }
        while (len)
            g.Put(tmp[--len]);
    }

    static void WriteInt(Gadget& g,
                         IntType value,
                         bool show_sign = false,
                         bool space_flag = false) noexcept {
        if (value < IntType(0)) {
            g.Put('-');
            // Negate via uint64_t arithmetic to avoid UB on IntType's minimum value.
            WriteUnsigned(g, static_cast<uint64_t>(-(static_cast<int64_t>(value) + 1)) + 1ULL);
        } else {
            if (show_sign)
                g.Put('+');
            else if (space_flag)
                g.Put(' ');
            WriteUnsigned(g, static_cast<uint64_t>(value));
        }
    }

    /// Compute FloatType(10^n) at compile time (n <= kMaxFloatPrecision).
    static constexpr FloatType Pow10(std::size_t n) noexcept {
        FloatType result{FloatType(1)};
        for (std::size_t i{0U}; i < n; ++i) {
            result = result * FloatType(10);
        }
        return result;
    }

    /// Integral and fractional decimal components of a floating-point value.
    struct FloatComponents {
        int64_t integral;
        int64_t fractional;
        bool is_negative;
    };

    /// Decompose a finite float into integral and fractional decimal components.
    ///
    /// Ported from the get_components() algorithm in eyalroz/printf (MIT):
    /// - Splits integral and fractional parts before scaling to avoid
    ///   precision loss on large integrals.
    /// - Applies banker's rounding (round-half-to-even).
    /// - Handles carry-over when the fractional part rounds up to 10^precision.
    static FloatComponents GetComponents(FloatType value, std::size_t precision) noexcept {
        FloatComponents c{};
        c.is_negative = value < FloatType(0);
        FloatType abs_val{c.is_negative ? -value : value};

        c.integral = static_cast<int64_t>(abs_val);
        FloatType scaled_remainder{(abs_val - static_cast<FloatType>(c.integral)) *
                                   Pow10(precision)};
        c.fractional = static_cast<int64_t>(scaled_remainder);

        FloatType remainder{scaled_remainder - static_cast<FloatType>(c.fractional)};
        constexpr FloatType kHalf{FloatType(0.5)};

        // Banker's rounding: round up if remainder > 0.5, or if == 0.5 and
        // the fractional part is odd (round-half-to-even).
        if (remainder > kHalf || (remainder == kHalf && (c.fractional & 1))) {
            ++c.fractional;
        }

        // Carry: fractional rounded up to 10^precision → propagate to integral.
        if (static_cast<FloatType>(c.fractional) >= Pow10(precision)) {
            c.fractional = 0;
            ++c.integral;
        }

        // For precision == 0 the fractional is always 0 (even), so the
        // banker's rounding above never fires for the half-way case.
        // Re-apply it directly on the integral part.
        if (precision == 0U) {
            remainder = abs_val - static_cast<FloatType>(c.integral);
            if (remainder == kHalf && (c.integral & 1)) {
                ++c.integral;
            }
        }

        return c;
    }

    // Largest FloatType value whose truncation to int64_t is defined behaviour.
    // cast<FloatType>(INT64_MAX) rounds up to 2^63; anything >= that overflows.
    static constexpr FloatType kMaxSafeIntegral = static_cast<FloatType>(INT64_MAX);

    static void WriteFloat(Gadget& g,
                           FloatType value,
                           std::size_t precision,
                           bool show_sign = false,
                           bool emit_sign = true,
                           bool alternate_form = false,
                           bool space_flag = false) noexcept {
        constexpr FloatType kFloatMax{std::numeric_limits<FloatType>::max()};
        if (value != value) {
            WriteRaw(g, "nan", 3U);
            return;
        }  // NaN — no sign
        if (value > kFloatMax) {
            if (emit_sign && show_sign)
                g.Put('+');
            else if (emit_sign && space_flag)
                g.Put(' ');
            WriteRaw(g, "inf", 3U);
            return;
        }
        if (value < -kFloatMax) {
            WriteRaw(g, "-inf", 4U);
            return;
        }  // -inf

        // Guard: abs value >= 2^63 would overflow int64_t in GetComponents.
        const FloatType abs_val{value < FloatType(0) ? -value : value};
        if (abs_val >= kMaxSafeIntegral) {
            if (emit_sign && value < FloatType(0))
                g.Put('-');
            else if (emit_sign && show_sign)
                g.Put('+');
            else if (emit_sign && space_flag)
                g.Put(' ');
            WriteRaw(g, "ovf", 3U);
            return;
        }

        if (precision > Policy::kMaxFloatPrecision)
            precision = Policy::kMaxFloatPrecision;

        const FloatComponents c = GetComponents(value, precision);

        if (emit_sign && c.is_negative)
            g.Put('-');
        else if (emit_sign && show_sign)
            g.Put('+');
        else if (emit_sign && space_flag && !c.is_negative)
            g.Put(' ');
        WriteUnsigned(g, static_cast<uint64_t>(c.integral));

        if (precision > 0U || alternate_form) {
            g.Put('.');
            // Build fractional digits right-to-left, emit left-to-right
            // so leading zeros are preserved.
            char frac[Policy::kMaxFloatPrecision > 0U ? Policy::kMaxFloatPrecision : 1U]{};
            uint64_t tmp{static_cast<uint64_t>(c.fractional)};
            for (std::size_t i = precision; i > 0U; --i) {
                frac[i - 1U] = static_cast<char>('0' + tmp % 10U);
                tmp /= 10U;
            }
            for (std::size_t i = 0U; i < precision; ++i)
                g.Put(frac[i]);
        }
    }

    // -------------------------------------------------------------------------
    // Length-modifier-aware argument readers
    // -------------------------------------------------------------------------

    static IntType ReadSignedArg(va_list& args, LengthMod len) noexcept {
        switch (len) {
            case LengthMod::hh:
                return static_cast<IntType>(static_cast<signed char>(va_arg(args, int)));
            case LengthMod::h:
                return static_cast<IntType>(static_cast<short>(va_arg(args, int)));
            case LengthMod::l:
                return static_cast<IntType>(va_arg(args, long));
            case LengthMod::ll:
                return static_cast<IntType>(va_arg(args, long long));
            case LengthMod::j:
                return static_cast<IntType>(va_arg(args, intmax_t));
            case LengthMod::z:
                return static_cast<IntType>(va_arg(args, size_t));
            case LengthMod::t:
                return static_cast<IntType>(va_arg(args, ptrdiff_t));
            default: {
                // Without a length modifier the variadic argument undergoes
                // default argument promotion to int.  We must read int to
                // avoid a va_arg type mismatch on platforms where the
                // policy's IntType is narrower than int (e.g. 16-bit int
                // targets with IntType = int32_t).
                //
                // When IntType is wider than int (e.g. HighPrecisionPolicy) the
                // caller must supply a length modifier (%ld, %lld, …);
                // otherwise the value is truncated to int-width.
                return static_cast<IntType>(va_arg(args, int));
            }
        }
    }

    static UIntType ReadUnsignedArg(va_list& args, LengthMod len) noexcept {
        switch (len) {
            case LengthMod::hh:
                return static_cast<UIntType>(static_cast<unsigned char>(va_arg(args, int)));
            case LengthMod::h:
                return static_cast<UIntType>(static_cast<unsigned short>(va_arg(args, int)));
            case LengthMod::l:
                return static_cast<UIntType>(va_arg(args, unsigned long));
            case LengthMod::ll:
                return static_cast<UIntType>(va_arg(args, unsigned long long));
            case LengthMod::j:
                return static_cast<UIntType>(va_arg(args, uintmax_t));
            case LengthMod::z:
                return static_cast<UIntType>(va_arg(args, size_t));
            case LengthMod::t:
                return static_cast<UIntType>(va_arg(args, ptrdiff_t));
            default: {
                // Without a length modifier the promoted type is unsigned int.
                // Reading the policy's UIntType directly would cause a va_arg
                // type mismatch when UIntType is not the same as unsigned int.
                return static_cast<UIntType>(va_arg(args, unsigned int));
            }
        }
    }

    static FloatType ReadFloatArg(va_list& args, LengthMod len) noexcept {
        if (len == LengthMod::L) {
            return static_cast<FloatType>(va_arg(args, long double));
        }
        return static_cast<FloatType>(va_arg(args, double));
    }

    // -------------------------------------------------------------------------
    // Specifier parsing (each helper advances @p fmt past what it consumes)
    // -------------------------------------------------------------------------

    /// Parse the flag characters. @c - / @c + / @c space / @c 0 / @c # are
    /// recognised; unknown characters stop parsing.
    static FormatFlags ParseFlags(const char*& fmt) noexcept {
        FormatFlags flags{};
        while (*fmt == '-' || *fmt == '+' || *fmt == ' ' || *fmt == '0' || *fmt == '#') {
            if (*fmt == '-')
                flags.left_justify = true;
            if (*fmt == '+')
                flags.show_sign = true;
            if (*fmt == ' ')
                flags.space_flag = true;
            if (*fmt == '0')
                flags.zero_pad = true;
            if (*fmt == '#')
                flags.alternate_form = true;
            ++fmt;
        }
        return flags;
    }

    /// Parse the field width. The first digit must be 1-9 (distinguishing width
    /// from the @c 0 flag); @c * reads the width from the argument list, and a
    /// negative @c * width forces left-justification (updating @p flags).
    static std::size_t ParseWidth(const char*& fmt, va_list& args, FormatFlags& flags) noexcept {
        std::size_t width{0U};
        if (*fmt == '*') {
            ++fmt;
            const int w{va_arg(args, int)};
            if (w < 0) {
                flags.left_justify = true;
                width = static_cast<std::size_t>(-w);
            } else {
                width = static_cast<std::size_t>(w);
            }
        } else if (*fmt >= '1' && *fmt <= '9') {
            width = static_cast<std::size_t>(*fmt++ - '0');
            while (*fmt >= '0' && *fmt <= '9')
                width = width * 10U + static_cast<std::size_t>(*fmt++ - '0');
        }
        return width;
    }

    /// Parse the @c .precision field. Absent precision yields the policy
    /// default; @c * reads it from the argument list (negative means "omitted").
    /// Values above @c Policy::kMaxFloatPrecision are clamped.
    static std::size_t ParsePrecision(const char*& fmt, va_list& args) noexcept {
        std::size_t precision{Policy::kDefaultFloatPrecision};
        if (*fmt == '.') {
            ++fmt;
            if (*fmt == '*') {
                ++fmt;
                const int p{va_arg(args, int)};
                if (p < 0)
                    precision = Policy::kDefaultFloatPrecision;
                else if (static_cast<std::size_t>(p) > Policy::kMaxFloatPrecision)
                    precision = Policy::kMaxFloatPrecision;
                else
                    precision = static_cast<std::size_t>(p);
            } else {
                precision = 0U;
                while (*fmt >= '0' && *fmt <= '9') {
                    precision = precision * 10U + static_cast<std::size_t>(*fmt++ - '0');
                    if (precision > Policy::kMaxFloatPrecision) {
                        precision = Policy::kMaxFloatPrecision;
                        while (*fmt >= '0' && *fmt <= '9')
                            ++fmt;  // drain remaining digits
                        break;
                    }
                }
            }
        }
        return precision;
    }

    /// Parse the length modifier (@c hh @c h @c l @c ll @c j @c z @c t @c L).
    static LengthMod ParseLengthMod(const char*& fmt) noexcept {
        switch (*fmt) {
            case 'h':
                ++fmt;
                if (*fmt == 'h') {
                    ++fmt;
                    return LengthMod::hh;
                }
                return LengthMod::h;
            case 'l':
                ++fmt;
                if (*fmt == 'l') {
                    ++fmt;
                    return LengthMod::ll;
                }
                return LengthMod::l;
            case 'j':
                ++fmt;
                return LengthMod::j;
            case 'z':
                ++fmt;
                return LengthMod::z;
            case 't':
                ++fmt;
                return LengthMod::t;
            case 'L':
                ++fmt;
                return LengthMod::L;
            default:
                return LengthMod::None;
        }
    }

    // -------------------------------------------------------------------------
    // Per-specifier formatting (apply width/precision/flags to a single value)
    // -------------------------------------------------------------------------

    /// @c %c — a single character, optionally space-padded to width.
    static void FormatChar(Gadget& g, const FormatSpec& spec, char c) noexcept {
        if (spec.width > 0U) {
            EmitSpacePadded(
                g, spec.width, spec.flags.left_justify, [c](Gadget& out) { out.Put(c); });
        } else {
            g.Put(c);
        }
    }

    /// @c %s — a null-terminated string, optionally space-padded to width.
    static void FormatStr(Gadget& g, const FormatSpec& spec, const char* s) noexcept {
        if (spec.width > 0U) {
            EmitSpacePadded(
                g, spec.width, spec.flags.left_justify, [s](Gadget& out) { WriteString(out, s); });
        } else {
            WriteString(g, s);
        }
    }

    /// @c %d / @c %i — signed decimal, honouring sign, width and zero-padding.
    static void FormatSignedInt(Gadget& g, const FormatSpec& spec, IntType v) noexcept {
        const bool show_sign{spec.flags.show_sign};
        const bool space_flag{spec.flags.space_flag};
        if (spec.width > 0U && spec.flags.zero_pad && !spec.flags.left_justify) {
            EmitZeroPadded(
                g,
                spec.width,
                [v, show_sign, space_flag](Gadget& out) {
                    if (v < IntType(0))
                        out.Put('-');
                    else if (show_sign)
                        out.Put('+');
                    else if (space_flag)
                        out.Put(' ');
                },
                [v](Gadget& out) { WriteUnsigned(out, AbsAsU64(v)); });
        } else if (spec.width > 0U) {
            EmitSpacePadded(
                g, spec.width, spec.flags.left_justify, [v, show_sign, space_flag](Gadget& out) {
                    WriteInt(out, v, show_sign, space_flag);
                });
        } else {
            WriteInt(g, v, show_sign, space_flag);
        }
    }

    /// @c %u — unsigned decimal, honouring width and zero-padding.
    static void FormatUnsignedInt(Gadget& g, const FormatSpec& spec, UIntType v) noexcept {
        if (spec.width > 0U && spec.flags.zero_pad && !spec.flags.left_justify) {
            EmitZeroPadded(
                g, spec.width, [](Gadget&) {}, [v](Gadget& out) { WriteUnsigned(out, v); });
        } else if (spec.width > 0U) {
            EmitSpacePadded(g, spec.width, spec.flags.left_justify, [v](Gadget& out) {
                WriteUnsigned(out, v);
            });
        } else {
            WriteUnsigned(g, v);
        }
    }

    /// @c %x / @c %X — hexadecimal, honouring the @c # prefix, width and
    /// zero-padding. @p uppercase selects the digit case and @c 0X prefix.
    static void FormatHex(Gadget& g, const FormatSpec& spec, UIntType v, bool uppercase) noexcept {
        const bool use_prefix{spec.flags.alternate_form && v != UIntType(0)};
        const char prefix_char{uppercase ? 'X' : 'x'};
        const auto write_prefix{[use_prefix, prefix_char](Gadget& out) {
            if (use_prefix) {
                out.Put('0');
                out.Put(prefix_char);
            }
        }};
        if (spec.width > 0U && spec.flags.zero_pad && !spec.flags.left_justify) {
            EmitZeroPadded(g, spec.width, write_prefix, [v, uppercase](Gadget& out) {
                WriteHex(out, v, uppercase);
            });
        } else if (spec.width > 0U) {
            EmitSpacePadded(
                g, spec.width, spec.flags.left_justify, [v, uppercase, write_prefix](Gadget& out) {
                    write_prefix(out);
                    WriteHex(out, v, uppercase);
                });
        } else {
            write_prefix(g);
            WriteHex(g, v, uppercase);
        }
    }

    /// @c %f — decimal float, honouring sign, precision, width and
    /// zero-padding. Zero-padding only applies to finite values; nan/inf/ovf
    /// fall back to space padding so their sign words stay intact.
    static void FormatFloatValue(Gadget& g, const FormatSpec& spec, FloatType v) noexcept {
        const bool show_sign{spec.flags.show_sign};
        const bool space_flag{spec.flags.space_flag};
        const bool alternate_form{spec.flags.alternate_form};
        const std::size_t precision{spec.precision};

        if (spec.width > 0U && spec.flags.zero_pad && !spec.flags.left_justify) {
            const bool is_normal{v == v && v >= -std::numeric_limits<FloatType>::max() &&
                                 v <= std::numeric_limits<FloatType>::max() &&
                                 v > -kMaxSafeIntegral && v < kMaxSafeIntegral};
            Gadget dry{MakeCountingGadget()};
            WriteFloat(dry, v, precision, show_sign, true, alternate_form, space_flag);
            if (is_normal) {
                if (v < FloatType(0))
                    g.Put('-');
                else if (show_sign)
                    g.Put('+');
                else if (space_flag)
                    g.Put(' ');
                EmitPadding(g, spec.width, dry.pos, '0');
                WriteFloat(g, v, precision, show_sign, false, alternate_form, space_flag);
            } else {
                EmitPadding(g, spec.width, dry.pos);
                WriteFloat(g, v, precision, show_sign, true, alternate_form, space_flag);
            }
        } else if (spec.width > 0U) {
            EmitSpacePadded(g,
                            spec.width,
                            spec.flags.left_justify,
                            [v, precision, show_sign, alternate_form, space_flag](Gadget& out) {
                                WriteFloat(
                                    out, v, precision, show_sign, true, alternate_form, space_flag);
                            });
        } else {
            WriteFloat(g, v, precision, show_sign, true, alternate_form, space_flag);
        }
    }

    /// Read the argument for @p fmt's specifier and format it into @p g.
    /// @p fmt points at the specifier letter.
    void DispatchSpecifier(Gadget& g,
                           const char* fmt,
                           va_list& args,
                           const FormatSpec& spec) noexcept {
        switch (*fmt) {
            case 'c': {
                // char promotes to int in variadic calls.
                const char c{static_cast<char>(va_arg(args, int))};
                FormatChar(g, spec, c);
                break;
            }
            case 's': {
                const char* s{va_arg(args, const char*)};
                FormatStr(g, spec, s);
                break;
            }
            case 'd':
            case 'i':
                FormatSignedInt(g, spec, ReadSignedArg(args, spec.len_mod));
                break;
            case 'u':
                FormatUnsignedInt(g, spec, ReadUnsignedArg(args, spec.len_mod));
                break;
            case 'x':
                FormatHex(g, spec, ReadUnsignedArg(args, spec.len_mod), false);
                break;
            case 'X':
                FormatHex(g, spec, ReadUnsignedArg(args, spec.len_mod), true);
                break;
            case 'f': {
                // Variadic args promote float→double; cast back to FloatType
                // for policy-controlled precision. Always consume the argument
                // to keep va_list aligned, but only format when the policy
                // permits.
                const FloatType v{ReadFloatArg(args, spec.len_mod)};
                if (Policy::kSupportFloatingPointDecimals) {
                    FormatFloatValue(g, spec, v);
                }
                break;
            }
            case '%':
                g.Put('%');
                break;
            default:
                // Unknown specifier: emit literally (%?)
                g.Put('%');
                g.Put(*fmt);
                break;
        }
    }

    // -------------------------------------------------------------------------
    // Core format loop
    // -------------------------------------------------------------------------

    std::size_t DoFormat(const char* fmt, va_list& args, std::size_t start_pos = 0U) noexcept {
        Gadget g{buffer_.data(), start_pos, N};

        while (*fmt) {
            if (*fmt != '%') {
                g.Put(*fmt++);
                continue;
            }

            ++fmt;  // consume '%'
            if (!*fmt)
                break;

            // Parse the conversion spec: %[flags][width][.precision][length].
            FormatSpec spec{};
            spec.flags = ParseFlags(fmt);
            spec.width = ParseWidth(fmt, args, spec.flags);
            spec.precision = ParsePrecision(fmt, args);
            spec.len_mod = ParseLengthMod(fmt);

            // fmt now points at the specifier letter.
            DispatchSpecifier(g, fmt, args, spec);
            ++fmt;
        }

        // Write null terminator into the reserved slot (buffer_ has N+1 chars).
        buffer_[g.pos < N ? g.pos : N] = '\0';
        return g.pos < N ? g.pos : N;
    }

    std::array<char, N + 1> buffer_{};
    std::size_t size_{0};
};

}  // namespace ffb
