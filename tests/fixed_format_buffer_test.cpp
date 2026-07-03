#include "ffb/fixed_format_buffer.h"

#include <gtest/gtest.h>

using ffb::FixedFormatBuffer;
using ffb::Int64Policy;

struct LongDoublePolicy {
    static constexpr bool kSupportFloatingPointDecimals = true;
    static constexpr std::size_t kDefaultFloatPrecision = 6U;
    static constexpr std::size_t kMaxFloatPrecision     = 6U;
    using IntType = int32_t;
    using UIntType = uint32_t;
    using FloatType = long double;
};

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
    buf.Format("%i", int32_t(-2147483647 - 1));
    EXPECT_EQ(buf.View(), "-2147483648");
}

// ---------------------------------------------------------------------------
// Format — %f
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatFloat_DefaultPrecision) {
    FixedFormatBuffer<64> buf;
    buf.Format("%f", 3.14f);
    EXPECT_EQ(buf.View(), "3.140000");
}

TEST(FixedFormatBuffer, FormatFloat_CustomPrecision) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.2f", 3.14159f);
    EXPECT_EQ(buf.View(), "3.14");
}

TEST(FixedFormatBuffer, FormatFloat_ZeroPrecision) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.0f", 2.7f);
    EXPECT_EQ(buf.View(), "3");  // > 0.5, rounds up
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.0f", 2.5f);
    EXPECT_EQ(buf.View(), "2");  // round-half-to-even: 2 is even
    buf.Format("%.0f", 1.5f);
    EXPECT_EQ(buf.View(), "2");  // round-half-to-even: 2 is even
}

TEST(FixedFormatBuffer, FormatFloat_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.2f", -1.5f);
    EXPECT_EQ(buf.View(), "-1.50");
}

TEST(FixedFormatBuffer, FormatFloat_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.2f", 0.0f);
    EXPECT_EQ(buf.View(), "0.00");
}

TEST(FixedFormatBuffer, FormatFloat_AlternateForm_ZeroPrecision) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#.0f", 42.0f);
    EXPECT_EQ(buf.View(), "42.");
}

TEST(FixedFormatBuffer, FormatFloat_AlternateForm_ZeroPrecision_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#.0f", -42.0f);
    EXPECT_EQ(buf.View(), "-42.");
}

TEST(FixedFormatBuffer, FormatFloat_AlternateForm_WithPrecision) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#.2f", 3.14f);
    EXPECT_EQ(buf.View(), "3.14");
}

TEST(FixedFormatBuffer, FormatFloat_AlternateForm_Width_RightJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#8.0f", 42.0f);
    EXPECT_EQ(buf.View(), "     42.");
}

TEST(FixedFormatBuffer, FormatFloat_AlternateForm_Width_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#-8.0f", 42.0f);
    EXPECT_EQ(buf.View(), "42.     ");
}

TEST(FixedFormatBuffer, FormatFloat_AlternateForm_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#08.0f", 42.0f);
    EXPECT_EQ(buf.View(), "0000042.");
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
// Length modifiers
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, LengthMod_hd_Short) {
    FixedFormatBuffer<64> buf;
    buf.Format("%hd", short(42));
    EXPECT_EQ(buf.View(), "42");
}

TEST(FixedFormatBuffer, LengthMod_hd_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%hd", short(-1));
    EXPECT_EQ(buf.View(), "-1");
}

TEST(FixedFormatBuffer, LengthMod_hhd_Char) {
    FixedFormatBuffer<64> buf;
    buf.Format("%hhd", static_cast<signed char>(-128));
    EXPECT_EQ(buf.View(), "-128");
}

TEST(FixedFormatBuffer, LengthMod_ld_Long) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%ld", 42L);
    EXPECT_EQ(buf.View(), "42");
}

TEST(FixedFormatBuffer, LengthMod_lld) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%lld", 42LL);
    EXPECT_EQ(buf.View(), "42");
}

TEST(FixedFormatBuffer, LengthMod_hu) {
    FixedFormatBuffer<64> buf;
    buf.Format("%hu", static_cast<unsigned short>(255));
    EXPECT_EQ(buf.View(), "255");
}

TEST(FixedFormatBuffer, LengthMod_lu) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%lu", 42UL);
    EXPECT_EQ(buf.View(), "42");
}

TEST(FixedFormatBuffer, LengthMod_llu) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%llu", 42ULL);
    EXPECT_EQ(buf.View(), "42");
}

TEST(FixedFormatBuffer, LengthMod_lx) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%lx", 0x2AUL);
    EXPECT_EQ(buf.View(), "2a");
}

TEST(FixedFormatBuffer, LengthMod_llX) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%llX", 0xFFULL);
    EXPECT_EQ(buf.View(), "FF");
}

TEST(FixedFormatBuffer, LengthMod_jd) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%jd", static_cast<intmax_t>(-1));
    EXPECT_EQ(buf.View(), "-1");
}

TEST(FixedFormatBuffer, LengthMod_ju) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%ju", static_cast<uintmax_t>(42));
    EXPECT_EQ(buf.View(), "42");
}

TEST(FixedFormatBuffer, LengthMod_zu) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%zu", static_cast<size_t>(100));
    EXPECT_EQ(buf.View(), "100");
}

TEST(FixedFormatBuffer, LengthMod_td) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%td", static_cast<ptrdiff_t>(-7));
    EXPECT_EQ(buf.View(), "-7");
}

TEST(FixedFormatBuffer, LengthMod_Lf) {
    FixedFormatBuffer<64, LongDoublePolicy> buf;
    buf.Format("%.1Lf", static_cast<long double>(3.1L));
    EXPECT_EQ(buf.View(), "3.1");
}

TEST(FixedFormatBuffer, LengthMod_Combined_FlagsAndLength) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%+#08llx", 0x2AULL);
    EXPECT_EQ(buf.View(), "0x00002a");
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
// * width and precision
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, StarWidth_Int) {
    FixedFormatBuffer<64> buf;
    buf.Format("%*d", 6, 42);
    EXPECT_EQ(buf.View(), "    42");
}

TEST(FixedFormatBuffer, StarWidth_Negative_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%*d", -6, 42);
    EXPECT_EQ(buf.View(), "42    ");
}

TEST(FixedFormatBuffer, StarPrecision_Float) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.*f", 2, 3.14159f);
    EXPECT_EQ(buf.View(), "3.14");
}

TEST(FixedFormatBuffer, StarWidthAndPrecision_Float) {
    FixedFormatBuffer<64> buf;
    buf.Format("%*.*f", 8, 2, 3.14159f);
    EXPECT_EQ(buf.View(), "    3.14");
}

TEST(FixedFormatBuffer, StarWidthAndPrecision_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-*.*f", 8, 2, 3.14159f);
    EXPECT_EQ(buf.View(), "3.14    ");
}

TEST(FixedFormatBuffer, StarPrecision_Negative_FallsBackToDefault) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.*f", -1, 3.14159f);
    EXPECT_EQ(buf.View(), "3.141590");
}

TEST(FixedFormatBuffer, StarWidth_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%0*d", 6, 42);
    EXPECT_EQ(buf.View(), "000042");
}

// ---------------------------------------------------------------------------
// Space flag (' ')
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, SpaceFlag_Int_Positive) {
    FixedFormatBuffer<64> buf;
    buf.Format("% d", 42);
    EXPECT_EQ(buf.View(), " 42");
}

TEST(FixedFormatBuffer, SpaceFlag_Int_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("% d", -7);
    EXPECT_EQ(buf.View(), "-7");
}

TEST(FixedFormatBuffer, SpaceFlag_Int_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("% d", 0);
    EXPECT_EQ(buf.View(), " 0");
}

TEST(FixedFormatBuffer, SpaceFlag_OverriddenByShowSign_Int) {
    FixedFormatBuffer<64> buf;
    buf.Format("% +d", 42);
    EXPECT_EQ(buf.View(), "+42");
}

TEST(FixedFormatBuffer, SpaceFlag_WithWidth_Int) {
    FixedFormatBuffer<64> buf;
    buf.Format("% 6d", 42);
    EXPECT_EQ(buf.View(), "    42");
}

TEST(FixedFormatBuffer, SpaceFlag_WithZeroPad_Int) {
    FixedFormatBuffer<64> buf;
    buf.Format("% 06d", 42);
    EXPECT_EQ(buf.View(), " 00042");
}

TEST(FixedFormatBuffer, SpaceFlag_Float_Positive) {
    FixedFormatBuffer<64> buf;
    buf.Format("% .1f", 3.1f);
    EXPECT_EQ(buf.View(), " 3.1");
}

TEST(FixedFormatBuffer, SpaceFlag_Float_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("% .1f", -3.1f);
    EXPECT_EQ(buf.View(), "-3.1");
}

TEST(FixedFormatBuffer, SpaceFlag_Float_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("% .1f", 0.0f);
    EXPECT_EQ(buf.View(), " 0.0");
}

TEST(FixedFormatBuffer, SpaceFlag_OverriddenByShowSign_Float) {
    FixedFormatBuffer<64> buf;
    buf.Format("% +.1f", 3.1f);
    EXPECT_EQ(buf.View(), "+3.1");
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

TEST(FixedFormatBuffer, FormatHex_AlternateForm) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#x", 0x2aU);
    EXPECT_EQ(buf.View(), "0x2a");
}

TEST(FixedFormatBuffer, FormatHex_AlternateForm_ZeroValue) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#x", 0U);
    EXPECT_EQ(buf.View(), "0");
}

TEST(FixedFormatBuffer, FormatHex_AlternateForm_Width_RightJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#8x", 0x2aU);
    EXPECT_EQ(buf.View(), "    0x2a");
}

TEST(FixedFormatBuffer, FormatHex_AlternateForm_Width_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#-8x", 0x2aU);
    EXPECT_EQ(buf.View(), "0x2a    ");
}

TEST(FixedFormatBuffer, FormatHex_AlternateForm_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#08x", 0x2aU);
    EXPECT_EQ(buf.View(), "0x00002a");
}

TEST(FixedFormatBuffer, FormatHex_AlternateForm_ZeroPad_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#06x", 0x2aU);
    EXPECT_EQ(buf.View(), "0x002a");
}

TEST(FixedFormatBuffer, FormatHex_AlternateForm_NarrowerThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#4x", 0x2afeU);
    EXPECT_EQ(buf.View(), "0x2afe");
}

// ---------------------------------------------------------------------------
// Format — %X (uppercase hex)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatHexUpper_Basic) {
    FixedFormatBuffer<64> buf;
    buf.Format("%X", 0x2afeU);
    EXPECT_EQ(buf.View(), "2AFE");
}

TEST(FixedFormatBuffer, FormatHexUpper_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%X", 0U);
    EXPECT_EQ(buf.View(), "0");
}

TEST(FixedFormatBuffer, FormatHexUpper_Width_RightJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%8X", 0x2aU);
    EXPECT_EQ(buf.View(), "      2A");
}

TEST(FixedFormatBuffer, FormatHexUpper_Width_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-8X", 0x2aU);
    EXPECT_EQ(buf.View(), "2A      ");
}

TEST(FixedFormatBuffer, FormatHexUpper_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08X", 0x2aU);
    EXPECT_EQ(buf.View(), "0000002A");
}

TEST(FixedFormatBuffer, FormatHexUpper_AlternateForm) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#X", 0x2aU);
    EXPECT_EQ(buf.View(), "0X2A");
}

TEST(FixedFormatBuffer, FormatHexUpper_AlternateForm_ZeroValue) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#X", 0U);
    EXPECT_EQ(buf.View(), "0");
}

TEST(FixedFormatBuffer, FormatHexUpper_AlternateForm_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#08X", 0x2aU);
    EXPECT_EQ(buf.View(), "0X00002A");
}

TEST(FixedFormatBuffer, FormatHexUpper_InSentence) {
    FixedFormatBuffer<64> buf;
    buf.Format("val=%X done", 0xffU);
    EXPECT_EQ(buf.View(), "val=FF done");
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
