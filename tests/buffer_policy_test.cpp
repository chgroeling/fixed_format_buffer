#include "ffb/fixed_format_buffer.h"

#include <cinttypes>
#include <cstdint>
#include <gtest/gtest.h>

using ffb::AllFeatures;
using ffb::FixedFormatBuffer;
using ffb::Int64Policy;

// Local policy for tests — float disabled.
struct NoFloat {
    static constexpr bool        kSupportFloatingPointDecimals = false;
    static constexpr std::size_t kDefaultFloatPrecision        = 6U;
    using IntType   = int32_t;
    using UIntType  = uint32_t;
    using FloatType = float;
};

// ---------------------------------------------------------------------------
// Policy instantiation smoke tests
// ---------------------------------------------------------------------------

TEST(BufferPolicy, DefaultPolicyInstantiates) {
    FixedFormatBuffer<64> buf;  // Policy = AllFeatures
    EXPECT_TRUE(buf.Empty());
}

TEST(BufferPolicy, ExplicitAllFeaturesPolicyInstantiates) {
    FixedFormatBuffer<64, AllFeatures> buf;
    EXPECT_TRUE(buf.Empty());
}

TEST(BufferPolicy, NoFloatPolicyInstantiates) {
    FixedFormatBuffer<64, NoFloat> buf;
    EXPECT_TRUE(buf.Empty());
}

// ---------------------------------------------------------------------------
// AllFeatures: float formatting works
// ---------------------------------------------------------------------------

TEST(BufferPolicy, AllFeaturesFormatsFloat) {
    FixedFormatBuffer<64, AllFeatures> buf;
    buf.Format("%.2f", 3.14f);
    EXPECT_EQ(buf.View(), "3.14");
}

// ---------------------------------------------------------------------------
// NoFloat: %f arg is consumed but produces no output; subsequent args OK
// ---------------------------------------------------------------------------

TEST(BufferPolicy, NoFloatSkipsFloatOutput) {
    FixedFormatBuffer<64, NoFloat> buf;
    buf.Format("%f", 3.14f);
    EXPECT_EQ(buf.View(), "");
}

TEST(BufferPolicy, NoFloatKeepsVaListAligned) {
    // %f is consumed silently; %s after it must still read the right arg.
    FixedFormatBuffer<64, NoFloat> buf;
    buf.Format("%f %s", 1.0f, "ok");
    EXPECT_EQ(buf.View(), " ok");
}

TEST(BufferPolicy, NoFloatAllowsIntAndString) {
    FixedFormatBuffer<64, NoFloat> buf;
    buf.Format("i=%i s=%s", 7, "hi");
    EXPECT_EQ(buf.View(), "i=7 s=hi");
}

// ---------------------------------------------------------------------------
// Int64Policy: 64-bit signed decimals
// ---------------------------------------------------------------------------

TEST(BufferPolicy, Int64_Max) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%lli", INT64_MAX);
    EXPECT_EQ(buf.View(), "9223372036854775807");
}

TEST(BufferPolicy, Int64_Min) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%lli", INT64_MIN);
    EXPECT_EQ(buf.View(), "-9223372036854775808");
}

TEST(BufferPolicy, Int64_BeyondInt32Max) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%lli", INT64_C(3000000000));
    EXPECT_EQ(buf.View(), "3000000000");
}

TEST(BufferPolicy, Int64_BeyondInt32Min) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%lli", INT64_C(-3000000000));
    EXPECT_EQ(buf.View(), "-3000000000");
}

// ---------------------------------------------------------------------------
// Int64Policy: 64-bit unsigned decimals
// ---------------------------------------------------------------------------

TEST(BufferPolicy, Uint64_BeyondUint32Max) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%llu", UINT64_C(5000000000));
    EXPECT_EQ(buf.View(), "5000000000");
}

TEST(BufferPolicy, Uint64_Max) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%llu", UINT64_C(18446744073709551615));
    EXPECT_EQ(buf.View(), "18446744073709551615");
}

// ---------------------------------------------------------------------------
// Int64Policy: 64-bit hex
// ---------------------------------------------------------------------------

TEST(BufferPolicy, Hex64_Large) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%llx", UINT64_C(0xDEADBEEFCAFEBABE));
    EXPECT_EQ(buf.View(), "deadbeefcafebabe");
}

TEST(BufferPolicy, Hex64_Max) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%llx", UINT64_C(0xFFFFFFFFFFFFFFFF));
    EXPECT_EQ(buf.View(), "ffffffffffffffff");
}

TEST(BufferPolicy, Hex64_ZeroPad) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%016llx", UINT64_C(0xCAFE));
    EXPECT_EQ(buf.View(), "000000000000cafe");
}

// ---------------------------------------------------------------------------
// 32-bit policy rejects wider types at compile time (formerly UB truncation)
//
// Passing a 64-bit integer to the default 32-bit policy is a compile error:
//   buf.Format("%i", INT64_C(42));  // sizeof(long long) > sizeof(int32_t)
// Use Int64Policy (see above) or cast to the policy's IntType / UIntType.
// ---------------------------------------------------------------------------
