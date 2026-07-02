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
    EXPECT_EQ(buf.View(), "3");  // > 0.5, rounds up
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.0f", 2.5);
    EXPECT_EQ(buf.View(), "2");  // round-half-to-even: 2 is even
    buf.Format("%.0f", 1.5);
    EXPECT_EQ(buf.View(), "2");  // round-half-to-even: 2 is even
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

// ---------------------------------------------------------------------------
// Format — width (right-align, space-padded)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, Width_Int_WiderThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%6d", 42);
    EXPECT_EQ(buf.View(), "    42");
}

TEST(FixedFormatBuffer, Width_Int_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%2d", 42);
    EXPECT_EQ(buf.View(), "42");
}

TEST(FixedFormatBuffer, Width_Int_NarrowerThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%2d", 12345);
    EXPECT_EQ(buf.View(), "12345"); // no truncation from width
}

TEST(FixedFormatBuffer, Width_Int_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%6d", -7);
    EXPECT_EQ(buf.View(), "    -7");
}

TEST(FixedFormatBuffer, Width_Float_WiderThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%12.2f", 3.14f);
    EXPECT_EQ(buf.View(), "        3.14");
}

TEST(FixedFormatBuffer, Width_Float_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%4.2f", 3.14f);
    EXPECT_EQ(buf.View(), "3.14");
}

TEST(FixedFormatBuffer, Width_Float_NarrowerThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%3.2f", 3.14f);
    EXPECT_EQ(buf.View(), "3.14"); // no truncation from width
}

TEST(FixedFormatBuffer, Width_Float_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%8.2f", -1.5f);
    EXPECT_EQ(buf.View(), "   -1.50");
}

TEST(FixedFormatBuffer, Width_Int_MultiDigitWidth) {
    FixedFormatBuffer<64> buf;
    buf.Format("%10d", 99);
    EXPECT_EQ(buf.View(), "        99");
}

// ---------------------------------------------------------------------------
// Format — + flag (show_sign)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, ShowSign_Int_Positive) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+d", 42);
    EXPECT_EQ(buf.View(), "+42");
}

TEST(FixedFormatBuffer, ShowSign_Int_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+d", -42);
    EXPECT_EQ(buf.View(), "-42");
}

TEST(FixedFormatBuffer, ShowSign_Int_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+d", 0);
    EXPECT_EQ(buf.View(), "+0");
}

TEST(FixedFormatBuffer, ShowSign_Int_WithWidth_RightJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+8d", 42);
    EXPECT_EQ(buf.View(), "     +42");
}

TEST(FixedFormatBuffer, ShowSign_Int_WithWidth_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-+8d", 42);
    EXPECT_EQ(buf.View(), "+42     ");
}

TEST(FixedFormatBuffer, ShowSign_Float_Positive) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+.2f", 1.5f);
    EXPECT_EQ(buf.View(), "+1.50");
}

TEST(FixedFormatBuffer, ShowSign_Float_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+.2f", -1.5f);
    EXPECT_EQ(buf.View(), "-1.50");
}

TEST(FixedFormatBuffer, ShowSign_Float_WithWidth) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+8.2f", 3.14f);
    EXPECT_EQ(buf.View(), "   +3.14");
}

TEST(FixedFormatBuffer, ShowSign_Float_WithWidth_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-+8.2f", 3.14f);
    EXPECT_EQ(buf.View(), "+3.14   ");
}

// ---------------------------------------------------------------------------
// Format — left-justify combined with forced sign (%-+)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, LeftJustifyShowSign_Int_Positive) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-+10d", 42);
    EXPECT_EQ(buf.View(), "+42       ");
}

TEST(FixedFormatBuffer, LeftJustifyShowSign_Int_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-+10d", -42);
    EXPECT_EQ(buf.View(), "-42       ");
}

TEST(FixedFormatBuffer, LeftJustifyShowSign_Int_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-+10d", 0);
    EXPECT_EQ(buf.View(), "+0        ");
}

TEST(FixedFormatBuffer, LeftJustifyShowSign_Float_Positive) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-+12.3f", 3.14f);
    EXPECT_EQ(buf.View(), "+3.140      ");
}

TEST(FixedFormatBuffer, LeftJustifyShowSign_Float_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-+12.3f", -3.14f);
    EXPECT_EQ(buf.View(), "-3.140      ");
}

// ---------------------------------------------------------------------------
// Format — %c (character)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatChar_Basic) {
    FixedFormatBuffer<64> buf;
    buf.Format("%c", 'A');
    EXPECT_EQ(buf.View(), "A");
}

TEST(FixedFormatBuffer, FormatChar_Special) {
    FixedFormatBuffer<64> buf;
    buf.Format("%c", '!');
    EXPECT_EQ(buf.View(), "!");
}

TEST(FixedFormatBuffer, FormatChar_InSentence) {
    FixedFormatBuffer<64> buf;
    buf.Format("x=%c y=%c", 'A', 'B');
    EXPECT_EQ(buf.View(), "x=A y=B");
}

TEST(FixedFormatBuffer, FormatChar_Width_RightJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%5c", 'X');
    EXPECT_EQ(buf.View(), "    X");
}

TEST(FixedFormatBuffer, FormatChar_Width_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-5c", 'X');
    EXPECT_EQ(buf.View(), "X    ");
}

TEST(FixedFormatBuffer, FormatChar_Width_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%1c", 'Z');
    EXPECT_EQ(buf.View(), "Z");
}
