#pragma once

#include <array>
#include <cfloat>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "ffb/buffer_policy.h"

namespace ffb {

/// Allocation-free fixed-capacity formatting buffer.
///
/// Usable on the stack, as a class member, or statically allocated.
/// Formatted string views are transient and valid only until the buffer
/// is modified, reused, or destroyed.
///
/// @tparam N      Maximum number of characters (excluding null terminator).
/// @tparam Policy Feature-flag policy struct (see buffer_policy.h).
///                Defaults to AllFeatures (all features enabled).
template <std::size_t N, typename Policy = AllFeatures>
class FixedFormatBuffer {
public:
    /// Maximum number of characters the buffer can hold (excluding null terminator).
    static constexpr std::size_t CAPACITY = N;

    FixedFormatBuffer() noexcept { buffer_[0] = '\0'; }

    /// Format into the buffer using a subset of printf-style specifiers.
    ///
    /// Supported specifiers:
    ///   - @c %s  — null-terminated string (@c const @c char*)
    ///   - @c %d, @c %i  — signed decimal integer (@c int)
    ///   - @c %f  — decimal float (@c double); only when @c Policy::kFloatSupport is true.
    ///             Optional precision: @c %.Nf  (default: 6 digits).
    ///
    /// Truncates silently if the result exceeds capacity.
    /// @return Number of characters written (excluding null terminator).
    std::size_t Format(const char* fmt, ...) noexcept {
        va_list args;
        va_start(args, fmt);
        size_ = DoFormat(fmt, args);
        va_end(args);
        return size_;
    }

    /// Clear the buffer contents.
    void Clear() noexcept {
        buffer_[0] = '\0';
        size_ = 0;
    }

    /// Return a string_view over the current contents.
    /// Valid only until the buffer is modified or destroyed.
    [[nodiscard]] std::string_view View() const noexcept {
        return {buffer_.data(), size_};
    }

    /// Return the number of characters currently stored.
    [[nodiscard]] std::size_t Size() const noexcept { return size_; }

    /// Return true if the buffer is empty.
    [[nodiscard]] bool Empty() const noexcept { return size_ == 0; }

private:
    // -------------------------------------------------------------------------
    // Output gadget
    //
    // Wraps the destination buffer. pos always increments even past max_chars,
    // giving snprintf-style "would have written" semantics for free.
    // -------------------------------------------------------------------------
    struct Gadget {
        char*       buf;
        std::size_t pos;
        std::size_t max_chars;

        void Put(char c) noexcept {
            if (pos < max_chars) buf[pos] = c;
            ++pos;
        }
    };

    // -------------------------------------------------------------------------
    // Low-level writers
    // -------------------------------------------------------------------------

    static void WriteRaw(Gadget& g, const char* s, std::size_t len) noexcept {
        for (std::size_t i = 0U; i < len; ++i) g.Put(s[i]);
    }

    static void WriteString(Gadget& g, const char* s) noexcept {
        if (!s) { WriteRaw(g, "(null)", 6U); return; }
        while (*s) g.Put(*s++);
    }

    /// Write an unsigned 64-bit integer in decimal.
    static void WriteUnsigned(Gadget& g, uint64_t value) noexcept {
        char tmp[20]; // 2^64 needs at most 20 decimal digits
        std::size_t len = 0U;
        if (value == 0ULL) { g.Put('0'); return; }
        while (value) {
            tmp[len++] = static_cast<char>('0' + value % 10U);
            value /= 10U;
        }
        while (len) g.Put(tmp[--len]); // reverse
    }

    static void WriteInt(Gadget& g, int32_t value) noexcept {
        if (value < 0) {
            g.Put('-');
            // Negate via unsigned arithmetic to avoid UB on INT_MIN.
            WriteUnsigned(g, static_cast<uint64_t>(
                static_cast<uint32_t>(-(value + 1)) + 1U));
        } else {
            WriteUnsigned(g, static_cast<uint64_t>(
                static_cast<uint32_t>(value)));
        }
    }

    static constexpr std::size_t kDefaultFloatPrecision = 6U;
    static constexpr std::size_t kMaxFloatPrecision     = 6U;

    /// Powers of 10 indexed by precision (0 .. kMaxFloatPrecision).
    static constexpr double kPow10Table[] = {
        1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0, 1000000.0
    };

    /// Integral and fractional decimal components of a floating-point value.
    struct FloatComponents {
        int64_t integral;
        int64_t fractional;
        bool    is_negative;
    };

    /// Decompose a finite double into integral and fractional decimal components.
    ///
    /// Ported from the get_components() algorithm in eyalroz/printf (MIT):
    /// - Splits integral and fractional parts before scaling to avoid
    ///   precision loss on large integrals.
    /// - Applies banker's rounding (round-half-to-even).
    /// - Handles carry-over when the fractional part rounds up to 10^precision.
    static FloatComponents GetComponents(double value, std::size_t precision) noexcept {
        FloatComponents c;
        c.is_negative = value < 0.0;
        double abs_val = c.is_negative ? -value : value;

        c.integral = static_cast<int64_t>(abs_val);
        double scaled_remainder = (abs_val - static_cast<double>(c.integral))
                                  * kPow10Table[precision];
        c.fractional = static_cast<int64_t>(scaled_remainder);

        double remainder = scaled_remainder - static_cast<double>(c.fractional);
        constexpr double kHalf = 0.5;

        // Banker's rounding: round up if remainder > 0.5, or if == 0.5 and
        // the fractional part is odd (round-half-to-even).
        if (remainder > kHalf ||
            (remainder == kHalf && (c.fractional & 1))) {
            ++c.fractional;
        }

        // Carry: fractional rounded up to 10^precision → propagate to integral.
        if (static_cast<double>(c.fractional) >= kPow10Table[precision]) {
            c.fractional = 0;
            ++c.integral;
        }

        // For precision == 0 the fractional is always 0 (even), so the
        // banker's rounding above never fires for the half-way case.
        // Re-apply it directly on the integral part.
        if (precision == 0U) {
            remainder = abs_val - static_cast<double>(c.integral);
            if (remainder == kHalf && (c.integral & 1)) {
                ++c.integral;
            }
        }

        return c;
    }

    static void WriteFloat(Gadget& g, double value, std::size_t precision) noexcept {
        if (value != value)   { WriteRaw(g, "nan",  3U); return; } // NaN
        if (value >  DBL_MAX) { WriteRaw(g, "inf",  3U); return; } // +inf
        if (value < -DBL_MAX) { WriteRaw(g, "-inf", 4U); return; } // -inf

        if (precision > kMaxFloatPrecision) precision = kMaxFloatPrecision;

        const FloatComponents c = GetComponents(value, precision);

        if (c.is_negative) g.Put('-');
        WriteUnsigned(g, static_cast<uint64_t>(c.integral));

        if (precision > 0U) {
            g.Put('.');
            // Build fractional digits right-to-left, emit left-to-right
            // so leading zeros are preserved.
            char frac[kMaxFloatPrecision];
            uint64_t tmp = static_cast<uint64_t>(c.fractional);
            for (std::size_t i = precision; i > 0U; --i) {
                frac[i - 1U] = static_cast<char>('0' + tmp % 10U);
                tmp /= 10U;
            }
            for (std::size_t i = 0U; i < precision; ++i) g.Put(frac[i]);
        }
    }

    // -------------------------------------------------------------------------
    // Core format loop
    // -------------------------------------------------------------------------

    std::size_t DoFormat(const char* fmt, va_list args) noexcept {
        Gadget g{buffer_.data(), 0U, N};

        while (*fmt) {
            if (*fmt != '%') { g.Put(*fmt++); continue; }

            ++fmt; // consume '%'
            if (!*fmt) break;

            // Optional precision: .digits  (e.g. "%.2f")
            std::size_t precision = kDefaultFloatPrecision;
            if (*fmt == '.') {
                ++fmt;
                precision = 0U;
                while (*fmt >= '0' && *fmt <= '9')
                    precision = precision * 10U + static_cast<std::size_t>(*fmt++ - '0');
            }

            switch (*fmt) {
                case 's':
                    WriteString(g, va_arg(args, const char*));
                    break;
                case 'd':
                case 'i':
                    WriteInt(g, va_arg(args, int32_t));
                    break;
                case 'f': {
                    // Always consume the argument to keep va_list aligned,
                    // but only format it when the policy permits.
                    const double v = va_arg(args, double);
                    if constexpr (Policy::kFloatSupport) {
                        WriteFloat(g, v, precision);
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
            ++fmt;
        }

        // Write null terminator into the reserved slot (buffer_ has N+1 chars).
        buffer_[g.pos < N ? g.pos : N] = '\0';
        return g.pos < N ? g.pos : N;
    }

    std::array<char, N + 1> buffer_{};
    std::size_t              size_{0};
};

}  // namespace ffb

