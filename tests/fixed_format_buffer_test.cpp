#include "ffb/fixed_format_buffer.h"

#include <gtest/gtest.h>

using ffb::FixedFormatBuffer;

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, DefaultConstructedIsEmpty) {
    FixedFormatBuffer<64> buf;
    EXPECT_TRUE(buf.Empty());
    EXPECT_EQ(buf.Size(), 0u);
    EXPECT_EQ(buf.View(), "");
}

// ---------------------------------------------------------------------------
// Clear
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, ClearResetsBuffer) {
    FixedFormatBuffer<64> buf;
    buf.Clear();
    EXPECT_TRUE(buf.Empty());
    EXPECT_EQ(buf.Size(), 0u);
    EXPECT_EQ(buf.View(), "");
}
