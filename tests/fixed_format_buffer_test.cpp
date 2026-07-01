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
// Format
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatPlainString) {
    FixedFormatBuffer<64> buf;
    buf.Format("hello");
    EXPECT_EQ(buf.View(), "hello");
    EXPECT_EQ(buf.Size(), 5u);
}

TEST(FixedFormatBuffer, FormatWithArguments) {
    FixedFormatBuffer<64> buf;
    buf.Format("value=%d", 42);
    EXPECT_EQ(buf.View(), "value=42");
}

TEST(FixedFormatBuffer, FormatTruncatesAtCapacity) {
    FixedFormatBuffer<4> buf;
    buf.Format("hello");
    EXPECT_EQ(buf.Size(), 4u);
    EXPECT_EQ(buf.View(), "hell");
}

TEST(FixedFormatBuffer, FormatReturnsWrittenCount) {
    FixedFormatBuffer<64> buf;
    std::size_t written = buf.Format("%s", "abc");
    EXPECT_EQ(written, 3u);
}

// ---------------------------------------------------------------------------
// Clear
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, ClearResetsBuffer) {
    FixedFormatBuffer<64> buf;
    buf.Format("data");
    buf.Clear();
    EXPECT_TRUE(buf.Empty());
    EXPECT_EQ(buf.Size(), 0u);
    EXPECT_EQ(buf.View(), "");
}

// ---------------------------------------------------------------------------
// Reuse
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, SecondFormatOverwritesPrevious) {
    FixedFormatBuffer<64> buf;
    buf.Format("first");
    buf.Format("second");
    EXPECT_EQ(buf.View(), "second");
}
