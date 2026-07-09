#include "ffb/fixed_format_buffer.h"

#include <cinttypes>
#include <cstdint>
#include <gtest/gtest.h>

using ffb::FixedFormatBuffer;
using ffb::HighPrecisionPolicy;
using ffb::NoFloatPolicy;
using ffb::StandardPolicy;

// Local policy for tests — float disabled.
struct NoFloat {
    static constexpr bool kSupportFloatingPointDecimals = false;
    static constexpr std::size_t kDefaultFloatPrecision = 6U;
    static constexpr std::size_t kMaxFloatPrecision = 6U;
    using IntType = int32_t;
    using UIntType = uint32_t;
    using FloatType = float;
};

// ---------------------------------------------------------------------------
// Policy instantiation smoke tests
// ---------------------------------------------------------------------------

TEST(BufferPolicy, DefaultPolicyInstantiates) {
    FixedFormatBuffer<64> buf;  // Policy = StandardPolicy
    EXPECT_TRUE(buf.Empty());
}

TEST(BufferPolicy, ExplicitStandardPolicyInstantiates) {
    FixedFormatBuffer<64, StandardPolicy> buf;
    EXPECT_TRUE(buf.Empty());
}

TEST(BufferPolicy, NoFloatPolicyInstantiates) {
    FixedFormatBuffer<64, NoFloat> buf;
    EXPECT_TRUE(buf.Empty());
}

// ---------------------------------------------------------------------------
// StandardPolicy: float formatting works
// ---------------------------------------------------------------------------

TEST(BufferPolicy, StandardPolicyFormatsFloat) {
    FixedFormatBuffer<64, StandardPolicy> buf;
    buf.Format("%.2f", 3.14f);
    EXPECT_STREQ(buf.CStr(), "3.14");
}

// ---------------------------------------------------------------------------
// NoFloat: %f arg is consumed but produces no output; subsequent args OK
// ---------------------------------------------------------------------------

TEST(BufferPolicy, NoFloatSkipsFloatOutput) {
    FixedFormatBuffer<64, NoFloat> buf;
    buf.Format("%f", 3.14f);
    EXPECT_STREQ(buf.CStr(), "");
}

TEST(BufferPolicy, NoFloatKeepsVaListAligned) {
    // %f is consumed silently; %s after it must still read the right arg.
    FixedFormatBuffer<64, NoFloat> buf;
    buf.Format("%f %s", 1.0f, "ok");
    EXPECT_STREQ(buf.CStr(), " ok");
}

TEST(BufferPolicy, NoFloatAllowsIntAndString) {
    FixedFormatBuffer<64, NoFloat> buf;
    buf.Format("i=%i s=%s", 7, "hi");
    EXPECT_STREQ(buf.CStr(), "i=7 s=hi");
}

// ---------------------------------------------------------------------------
// ffb::NoFloatPolicy (library-provided): %f consumed but not emitted
// ---------------------------------------------------------------------------

TEST(BufferPolicy, NoFloatPolicyInstantiatesLibrary) {
    FixedFormatBuffer<64, NoFloatPolicy> buf;
    EXPECT_TRUE(buf.Empty());
}

TEST(BufferPolicy, NoFloatPolicySkipsFloatOutput) {
    FixedFormatBuffer<64, NoFloatPolicy> buf;
    buf.Format("%f", 3.14f);
    EXPECT_STREQ(buf.CStr(), "");
}

TEST(BufferPolicy, NoFloatPolicyKeepsVaListAligned) {
    // %f is consumed silently; %s after it must still read the right arg.
    FixedFormatBuffer<64, NoFloatPolicy> buf;
    buf.Format("%f %s", 1.0f, "ok");
    EXPECT_STREQ(buf.CStr(), " ok");
}

TEST(BufferPolicy, NoFloatPolicyIgnoresWidthOnFloat) {
    // Width/precision are parsed but the %f produces no output at all.
    FixedFormatBuffer<64, NoFloatPolicy> buf;
    buf.Format("[%10.2f]", 3.14f);
    EXPECT_STREQ(buf.CStr(), "[]");
}

TEST(BufferPolicy, NoFloatPolicyAllowsIntAndString) {
    FixedFormatBuffer<64, NoFloatPolicy> buf;
    buf.Format("i=%i s=%s", 7, "hi");
    EXPECT_STREQ(buf.CStr(), "i=7 s=hi");
}

TEST(BufferPolicy, NoFloatPolicyFormatsIntBetweenFloats) {
    FixedFormatBuffer<64, NoFloatPolicy> buf;
    buf.Format("%f%d%f", 1.0f, 42, 2.0f);
    EXPECT_STREQ(buf.CStr(), "42");
}

// ---------------------------------------------------------------------------
// HighPrecisionPolicy: 64-bit signed decimals
// ---------------------------------------------------------------------------

TEST(BufferPolicy, Int64_Max) {
    FixedFormatBuffer<64, HighPrecisionPolicy> buf;
    buf.Format("%lli", INT64_MAX);
    EXPECT_STREQ(buf.CStr(), "9223372036854775807");
}

TEST(BufferPolicy, Int64_Min) {
    FixedFormatBuffer<64, HighPrecisionPolicy> buf;
    buf.Format("%lli", INT64_MIN);
    EXPECT_STREQ(buf.CStr(), "-9223372036854775808");
}

TEST(BufferPolicy, Int64_BeyondInt32Max) {
    FixedFormatBuffer<64, HighPrecisionPolicy> buf;
    buf.Format("%lli", INT64_C(3000000000));
    EXPECT_STREQ(buf.CStr(), "3000000000");
}

TEST(BufferPolicy, Int64_BeyondInt32Min) {
    FixedFormatBuffer<64, HighPrecisionPolicy> buf;
    buf.Format("%lli", INT64_C(-3000000000));
    EXPECT_STREQ(buf.CStr(), "-3000000000");
}

// ---------------------------------------------------------------------------
// HighPrecisionPolicy: 64-bit unsigned decimals
// ---------------------------------------------------------------------------

TEST(BufferPolicy, Uint64_BeyondUint32Max) {
    FixedFormatBuffer<64, HighPrecisionPolicy> buf;
    buf.Format("%llu", UINT64_C(5000000000));
    EXPECT_STREQ(buf.CStr(), "5000000000");
}

TEST(BufferPolicy, Uint64_Max) {
    FixedFormatBuffer<64, HighPrecisionPolicy> buf;
    buf.Format("%llu", UINT64_C(18446744073709551615));
    EXPECT_STREQ(buf.CStr(), "18446744073709551615");
}

// ---------------------------------------------------------------------------
// HighPrecisionPolicy: 64-bit hex
// ---------------------------------------------------------------------------

TEST(BufferPolicy, Hex64_Large) {
    FixedFormatBuffer<64, HighPrecisionPolicy> buf;
    buf.Format("%llx", UINT64_C(0xDEADBEEFCAFEBABE));
    EXPECT_STREQ(buf.CStr(), "deadbeefcafebabe");
}

TEST(BufferPolicy, Hex64_Max) {
    FixedFormatBuffer<64, HighPrecisionPolicy> buf;
    buf.Format("%llx", UINT64_C(0xFFFFFFFFFFFFFFFF));
    EXPECT_STREQ(buf.CStr(), "ffffffffffffffff");
}

TEST(BufferPolicy, Hex64_ZeroPad) {
    FixedFormatBuffer<64, HighPrecisionPolicy> buf;
    buf.Format("%016llx", UINT64_C(0xCAFE));
    EXPECT_STREQ(buf.CStr(), "000000000000cafe");
}

// ---------------------------------------------------------------------------
// HighPrecisionPolicy: double-precision float formatting
// ---------------------------------------------------------------------------

TEST(BufferPolicy, DoubleFloat_Basic) {
    FixedFormatBuffer<64, HighPrecisionPolicy> buf;
    buf.Format("%.2f", 3.14);  // double literal, no 'f' suffix
    EXPECT_STREQ(buf.CStr(), "3.14");
}

TEST(BufferPolicy, DoubleFloat_Negative) {
    FixedFormatBuffer<64, HighPrecisionPolicy> buf;
    buf.Format("%.3f", -2.718);  // double literal
    EXPECT_STREQ(buf.CStr(), "-2.718");
}

TEST(BufferPolicy, DoubleFloat_DefaultPrecision) {
    FixedFormatBuffer<64, HighPrecisionPolicy> buf;
    buf.Format("%f", 1.0);  // double literal
    EXPECT_STREQ(buf.CStr(), "1.000000");
}

TEST(BufferPolicy, DoubleFloat_AlternateForm) {
    FixedFormatBuffer<64, HighPrecisionPolicy> buf;
    buf.Format("%#.0f", 42.0);  // double literal
    EXPECT_STREQ(buf.CStr(), "42.");
}

TEST(BufferPolicy, DoubleFloat_WidthAndSign) {
    FixedFormatBuffer<64, HighPrecisionPolicy> buf;
    buf.Format("%+10.2f", 1.5);  // double literal
    EXPECT_STREQ(buf.CStr(), "     +1.50");
}

// ---------------------------------------------------------------------------
// Proof that HighPrecisionPolicy uses double, not float, internally.
//
// A float has ~7 significant decimal digits; a double has ~15.
// At precision 9 the two produce different rounding for π:
//   float  π ≈ 3.141592736 → "3.141592736" at %.9f
//   double π ≈ 3.141592654 → "3.141592654" at %.9f
//
// The test uses a custom policy with kMaxFloatPrecision=9 because
// HighPrecisionPolicy caps at 6.  The FloatType = double is the key —
// replacing it with float would change the output to "3.141592741".
// ---------------------------------------------------------------------------

struct HighPrecisionDoublePolicy {
    static constexpr bool kSupportFloatingPointDecimals = true;
    static constexpr std::size_t kDefaultFloatPrecision = 9U;
    static constexpr std::size_t kMaxFloatPrecision = 9U;
    using IntType = int32_t;
    using UIntType = uint32_t;
    using FloatType = double;
};

struct HighPrecisionFloatPolicy {
    static constexpr bool kSupportFloatingPointDecimals = true;
    static constexpr std::size_t kDefaultFloatPrecision = 9U;
    static constexpr std::size_t kMaxFloatPrecision = 9U;
    using IntType = int32_t;
    using UIntType = uint32_t;
    using FloatType = float;
};

TEST(BufferPolicy, DoublePrecision_ProducesDifferentOutputThanFloat) {
    FixedFormatBuffer<64, HighPrecisionDoublePolicy> buf_double;
    FixedFormatBuffer<64, HighPrecisionFloatPolicy> buf_float;

    const double pi{3.14159265358979323846};

    buf_double.Format("%.9f", pi);
    buf_float.Format("%.9f", static_cast<float>(pi));

    EXPECT_STREQ(buf_double.CStr(), "3.141592654");
    EXPECT_STREQ(buf_float.CStr(), "3.141592736");
    EXPECT_STRNE(buf_double.CStr(), buf_float.CStr());
}

// ---------------------------------------------------------------------------
// 32-bit policy rejects wider types at compile time (formerly UB truncation)
//
// Passing a 64-bit integer to the default 32-bit policy is a compile error:
//   buf.Format("%i", INT64_C(42));  // sizeof(long long) > sizeof(int32_t)
// Use HighPrecisionPolicy (see above) or cast to the policy's IntType / UIntType.
// ---------------------------------------------------------------------------
