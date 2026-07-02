#include "ffb/buffer_policy.h"
#include "ffb/fixed_format_buffer.h"

#include <gtest/gtest.h>

using ffb::AllFeatures;
using ffb::FixedFormatBuffer;

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

// Local policy — float disabled.
struct NoFloat {
    static constexpr bool kFloatSupport = false;
};

TEST(BufferPolicy, NoFloatPolicyInstantiates) {
    FixedFormatBuffer<64, NoFloat> buf;
    EXPECT_TRUE(buf.Empty());
}
