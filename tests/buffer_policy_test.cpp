#include "ffb/buffer_policy.h"
#include "ffb/fixed_format_buffer.h"

#include <gtest/gtest.h>

using ffb::AllFeatures;
using ffb::FixedFormatBuffer;

// Local policy for tests — float disabled.
struct NoFloat {
    static constexpr bool kSupportFloatingPointDecimals = false;
    using IntType   = int;
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
    buf.Format("%.2f", 3.14);
    EXPECT_EQ(buf.View(), "3.14");
}

// ---------------------------------------------------------------------------
// NoFloat: %f arg is consumed but produces no output; subsequent args OK
// ---------------------------------------------------------------------------

TEST(BufferPolicy, NoFloatSkipsFloatOutput) {
    FixedFormatBuffer<64, NoFloat> buf;
    buf.Format("%f", 3.14);
    EXPECT_EQ(buf.View(), "");
}

TEST(BufferPolicy, NoFloatKeepsVaListAligned) {
    // %f is consumed silently; %s after it must still read the right arg.
    FixedFormatBuffer<64, NoFloat> buf;
    buf.Format("%f %s", 1.0, "ok");
    EXPECT_EQ(buf.View(), " ok");
}

TEST(BufferPolicy, NoFloatAllowsIntAndString) {
    FixedFormatBuffer<64, NoFloat> buf;
    buf.Format("i=%i s=%s", 7, "hi");
    EXPECT_EQ(buf.View(), "i=7 s=hi");
}
