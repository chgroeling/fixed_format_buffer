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
// Copy / assignment
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, CopyConstructor) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "hello");
    FixedFormatBuffer<64> copy{buf};
    EXPECT_EQ(copy.View(), "hello");
    EXPECT_EQ(copy.Size(), 5u);
    // Modifying the original does not affect the copy
    buf.Format("%s", "world");
    EXPECT_EQ(copy.View(), "hello");
}

TEST(FixedFormatBuffer, CopyAssignment) {
    FixedFormatBuffer<64> buf1;
    FixedFormatBuffer<64> buf2;
    buf1.Format("%s", "first");
    buf2.Format("%s", "second");
    buf2 = buf1;
    EXPECT_EQ(buf2.View(), "first");
    // Self-assignment safe
    buf2 = buf2;
    EXPECT_EQ(buf2.View(), "first");
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
// Format — 0 flag (zero-pad)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, ZeroPad_Int_WiderThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08d", 42);
    EXPECT_EQ(buf.View(), "00000042");
}

TEST(FixedFormatBuffer, ZeroPad_Int_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%02d", 42);
    EXPECT_EQ(buf.View(), "42");
}

TEST(FixedFormatBuffer, ZeroPad_Int_NarrowerThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%02d", 12345);
    EXPECT_EQ(buf.View(), "12345");
}

TEST(FixedFormatBuffer, ZeroPad_Int_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08d", -7);
    EXPECT_EQ(buf.View(), "-0000007");
}

TEST(FixedFormatBuffer, ZeroPad_Int_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%06d", 0);
    EXPECT_EQ(buf.View(), "000000");
}

TEST(FixedFormatBuffer, ZeroPad_Int_WithShowSign) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+08d", 42);
    EXPECT_EQ(buf.View(), "+0000042");
}

TEST(FixedFormatBuffer, ZeroPad_Int_NegativeWithShowSign) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+08d", -42);
    EXPECT_EQ(buf.View(), "-0000042");
}

TEST(FixedFormatBuffer, ZeroPad_Int_LeftJustifyOverridesZero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-08d", 42);
    EXPECT_EQ(buf.View(), "42      ");
}

TEST(FixedFormatBuffer, ZeroPad_Float_WiderThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08.2f", 3.14f);
    EXPECT_EQ(buf.View(), "00003.14");
}

TEST(FixedFormatBuffer, ZeroPad_Float_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%04.2f", 3.14f);
    EXPECT_EQ(buf.View(), "3.14");
}

TEST(FixedFormatBuffer, ZeroPad_Float_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08.2f", -1.5f);
    EXPECT_EQ(buf.View(), "-0001.50");
}

TEST(FixedFormatBuffer, ZeroPad_Float_WithShowSign) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+08.2f", 1.5f);
    EXPECT_EQ(buf.View(), "+0001.50");
}

TEST(FixedFormatBuffer, ZeroPad_Float_LeftJustifyOverridesZero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-08.2f", 3.14f);
    EXPECT_EQ(buf.View(), "3.14    ");
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

// ---------------------------------------------------------------------------
// Format — %x (hex)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatHex_Basic) {
    FixedFormatBuffer<64> buf;
    buf.Format("%x", 0x2aU);
    EXPECT_EQ(buf.View(), "2a");
}

TEST(FixedFormatBuffer, FormatHex_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%x", 0U);
    EXPECT_EQ(buf.View(), "0");
}

TEST(FixedFormatBuffer, FormatHex_MaxUint32) {
    FixedFormatBuffer<64> buf;
    buf.Format("%x", 0xFFFFFFFFU);
    EXPECT_EQ(buf.View(), "ffffffff");
}

TEST(FixedFormatBuffer, FormatHex_LowercaseDigits) {
    FixedFormatBuffer<64> buf;
    buf.Format("%x", 0xABCDEFU);
    EXPECT_EQ(buf.View(), "abcdef");
}

TEST(FixedFormatBuffer, FormatHex_Width_RightJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%8x", 0x2aU);
    EXPECT_EQ(buf.View(), "      2a");
}

TEST(FixedFormatBuffer, FormatHex_Width_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-8x", 0x2aU);
    EXPECT_EQ(buf.View(), "2a      ");
}

TEST(FixedFormatBuffer, FormatHex_Width_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%2x", 0x2aU);
    EXPECT_EQ(buf.View(), "2a");
}

TEST(FixedFormatBuffer, FormatHex_Width_NarrowerThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%2x", 0xabcU);
    EXPECT_EQ(buf.View(), "abc");
}

TEST(FixedFormatBuffer, FormatHex_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08x", 0x2aU);
    EXPECT_EQ(buf.View(), "0000002a");
}

TEST(FixedFormatBuffer, FormatHex_ZeroPad_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%02x", 0x2aU);
    EXPECT_EQ(buf.View(), "2a");
}

TEST(FixedFormatBuffer, FormatHex_LeftJustifyOverridesZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-08x", 0x2aU);
    EXPECT_EQ(buf.View(), "2a      ");
}

TEST(FixedFormatBuffer, FormatHex_InSentence) {
    FixedFormatBuffer<64> buf;
    buf.Format("val=0x%x done", 0xffU);
    EXPECT_EQ(buf.View(), "val=0xff done");
}

// ---------------------------------------------------------------------------
// Format — %u (unsigned decimal)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatUnsigned_Basic) {
    FixedFormatBuffer<64> buf;
    buf.Format("%u", 42U);
    EXPECT_EQ(buf.View(), "42");
}

TEST(FixedFormatBuffer, FormatUnsigned_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%u", 0U);
    EXPECT_EQ(buf.View(), "0");
}

TEST(FixedFormatBuffer, FormatUnsigned_MaxUint32) {
    FixedFormatBuffer<64> buf;
    buf.Format("%u", 0xFFFFFFFFU);
    EXPECT_EQ(buf.View(), "4294967295");
}

TEST(FixedFormatBuffer, FormatUnsigned_Width_RightJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%8u", 42U);
    EXPECT_EQ(buf.View(), "      42");
}

TEST(FixedFormatBuffer, FormatUnsigned_Width_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-8u", 42U);
    EXPECT_EQ(buf.View(), "42      ");
}

TEST(FixedFormatBuffer, FormatUnsigned_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08u", 42U);
    EXPECT_EQ(buf.View(), "00000042");
}

TEST(FixedFormatBuffer, FormatUnsigned_ZeroPad_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%02u", 42U);
    EXPECT_EQ(buf.View(), "42");
}

TEST(FixedFormatBuffer, FormatUnsigned_LeftJustifyOverridesZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-08u", 42U);
    EXPECT_EQ(buf.View(), "42      ");
}
