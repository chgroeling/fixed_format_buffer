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
// Format — %s
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatString) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "hello");
    EXPECT_EQ(buf.View(), "hello");
    EXPECT_EQ(buf.Size(), 5u);
}

TEST(FixedFormatBuffer, FormatNullString) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", nullptr);
    EXPECT_EQ(buf.View(), "(null)");
}

// ---------------------------------------------------------------------------
// Format — %i / %d
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatPositiveInt) {
    FixedFormatBuffer<64> buf;
    buf.Format("%i", 42);
    EXPECT_EQ(buf.View(), "42");
}

TEST(FixedFormatBuffer, FormatNegativeInt) {
    FixedFormatBuffer<64> buf;
    buf.Format("%i", -7);
    EXPECT_EQ(buf.View(), "-7");
}

TEST(FixedFormatBuffer, FormatZero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%d", 0);
    EXPECT_EQ(buf.View(), "0");
}

TEST(FixedFormatBuffer, FormatIntMin) {
    FixedFormatBuffer<64> buf;
    buf.Format("%i", -2147483648);
    EXPECT_EQ(buf.View(), "-2147483648");
}

// ---------------------------------------------------------------------------
// Format — %f
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatFloat_DefaultPrecision) {
    FixedFormatBuffer<64> buf;
    buf.Format("%f", 3.14);
    EXPECT_EQ(buf.View(), "3.140000");
}

TEST(FixedFormatBuffer, FormatFloat_CustomPrecision) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.2f", 3.14159);
    EXPECT_EQ(buf.View(), "3.14");
}

TEST(FixedFormatBuffer, FormatFloat_ZeroPrecision) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.0f", 2.7);
    EXPECT_EQ(buf.View(), "3");
}

TEST(FixedFormatBuffer, FormatFloat_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.2f", -1.5);
    EXPECT_EQ(buf.View(), "-1.50");
}

TEST(FixedFormatBuffer, FormatFloat_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.2f", 0.0);
    EXPECT_EQ(buf.View(), "0.00");
}

// ---------------------------------------------------------------------------
// Format — mixed args and literal text
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatMixedArgs) {
    FixedFormatBuffer<64> buf;
    buf.Format("x=%i, s=%s", 7, "hi");
    EXPECT_EQ(buf.View(), "x=7, s=hi");
}

TEST(FixedFormatBuffer, FormatLiteralPercent) {
    FixedFormatBuffer<64> buf;
    buf.Format("100%%");
    EXPECT_EQ(buf.View(), "100%");
}

// ---------------------------------------------------------------------------
// Format — truncation and return value
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatTruncatesAtCapacity) {
    FixedFormatBuffer<4> buf;
    buf.Format("%s", "hello");
    EXPECT_EQ(buf.Size(), 4u);
    EXPECT_EQ(buf.View(), "hell");
}

TEST(FixedFormatBuffer, FormatReturnsWrittenCount) {
    FixedFormatBuffer<64> buf;
    EXPECT_EQ(buf.Format("%s", "abc"), 3u);
}

TEST(FixedFormatBuffer, SecondFormatOverwritesPrevious) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "first");
    buf.Format("%s", "second");
    EXPECT_EQ(buf.View(), "second");
}

// ---------------------------------------------------------------------------
// Clear
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, ClearResetsBuffer) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "data");
    buf.Clear();
    EXPECT_TRUE(buf.Empty());
    EXPECT_EQ(buf.Size(), 0u);
    EXPECT_EQ(buf.View(), "");
}
