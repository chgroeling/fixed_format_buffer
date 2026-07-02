#include "ffb/buffer_policy.h"
#include "ffb/fixed_format_buffer.h"

#include <gtest/gtest.h>

using ffb::AllFeatures;
using ffb::FixedFormatBuffer;

// Local policy for tests — float disabled.
struct NoFloat {
    static constexpr bool kFloatSupport = false;
};

// ---------------------------------------------------------------------------
// AllFeatures policy (default)
// ---------------------------------------------------------------------------

TEST(BufferPolicy, DefaultPolicyAllowsFloat) {
    FixedFormatBuffer<64> buf;  // Policy = AllFeatures
    buf.Format("pi=%.2f", 3.14);
    EXPECT_EQ(buf.View(), "pi=3.14");
}

TEST(BufferPolicy, ExplicitAllFeaturesPolicyAllowsFloat) {
    FixedFormatBuffer<64, AllFeatures> buf;
    buf.Format("%.1f", 2.5f);
    EXPECT_EQ(buf.View(), "2.5");
}

TEST(BufferPolicy, AllFeaturesPolicyAllowsDouble) {
    FixedFormatBuffer<64, AllFeatures> buf;
    buf.Format("%.3f", 1.234);
    EXPECT_EQ(buf.View(), "1.234");
}

// ---------------------------------------------------------------------------
// NoFloat policy — non-float formatting must still work
// ---------------------------------------------------------------------------

TEST(BufferPolicy, NoFloatPolicyAllowsInteger) {
    FixedFormatBuffer<64, NoFloat> buf;
    buf.Format("x=%d", 42);
    EXPECT_EQ(buf.View(), "x=42");
}

TEST(BufferPolicy, NoFloatPolicyAllowsString) {
    FixedFormatBuffer<64, NoFloat> buf;
    buf.Format("%s", "hello");
    EXPECT_EQ(buf.View(), "hello");
}

TEST(BufferPolicy, NoFloatPolicyAllowsPlainString) {
    FixedFormatBuffer<64, NoFloat> buf;
    buf.Format("plain");
    EXPECT_EQ(buf.View(), "plain");
}

// ---------------------------------------------------------------------------
// Negative compile-time test (manual / documentation)
//
// The following code must NOT compile when Policy::kFloatSupport == false.
// It is left here as documentation and can be verified with a try_compile
// CMake test or by manually uncommenting:
//
//   FixedFormatBuffer<64, NoFloat> buf;
//   buf.Format("%.2f", 3.14f);  // static_assert: kFloatSupport = false
//
// ---------------------------------------------------------------------------
