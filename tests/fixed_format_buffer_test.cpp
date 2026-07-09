#include "ffb/fixed_format_buffer.h"

#include <limits>

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
    EXPECT_STREQ(buf.CStr(), "");
}

// ---------------------------------------------------------------------------
// Format — %s
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatString) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "hello");
    EXPECT_STREQ(buf.CStr(), "hello");
    EXPECT_EQ(buf.Size(), 5u);
}

TEST(FixedFormatBuffer, FormatNullString) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", nullptr);
    EXPECT_STREQ(buf.CStr(), "(null)");
}

// ---------------------------------------------------------------------------
// Format — %i / %d
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatPositiveInt) {
    FixedFormatBuffer<64> buf;
    buf.Format("%i", 42);
    EXPECT_STREQ(buf.CStr(), "42");
}

TEST(FixedFormatBuffer, FormatNegativeInt) {
    FixedFormatBuffer<64> buf;
    buf.Format("%i", -7);
    EXPECT_STREQ(buf.CStr(), "-7");
}

TEST(FixedFormatBuffer, FormatZero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%d", 0);
    EXPECT_STREQ(buf.CStr(), "0");
}

TEST(FixedFormatBuffer, FormatIntMin) {
    FixedFormatBuffer<64> buf;
    buf.Format("%i", int32_t(-2147483647 - 1));
    EXPECT_STREQ(buf.CStr(), "-2147483648");
}

// ---------------------------------------------------------------------------
// Format — %f
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatFloat_DefaultPrecision) {
    FixedFormatBuffer<64> buf;
    buf.Format("%f", 3.14f);
    EXPECT_STREQ(buf.CStr(), "3.140000");
}

TEST(FixedFormatBuffer, FormatFloat_CustomPrecision) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.2f", 3.14159f);
    EXPECT_STREQ(buf.CStr(), "3.14");
}

TEST(FixedFormatBuffer, FormatFloat_ZeroPrecision) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.0f", 2.7f);
    EXPECT_STREQ(buf.CStr(), "3");  // > 0.5, rounds up
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding_Precision0_EvenInteger) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.0f", 2.5f);
    EXPECT_STREQ(buf.CStr(), "2");  // 2.5→2 (2 is even, tie breaks to even)
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding_Precision0_OddInteger) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.0f", 1.5f);
    EXPECT_STREQ(buf.CStr(), "2");  // 1.5→2 (1 is odd, tie breaks up to even)
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding_Precision1_EvenLastDigit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.1f", 2.25f);  // exactly 2.25 in binary (9/4)
    EXPECT_STREQ(buf.CStr(), "2.2");  // .25 → scaled frac=2 even, tie stays
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding_Precision1_OddLastDigit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.1f", 2.75f);  // exactly 2.75 in binary (11/4)
    EXPECT_STREQ(buf.CStr(), "2.8");  // .75 → scaled frac=7 odd, tie rounds up to 8
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding_Precision2_EvenLastDigit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.2f", 2.125f);  // exactly 2.125 in binary (17/8)
    EXPECT_STREQ(buf.CStr(), "2.12");  // .125 → scaled frac=12 even, tie stays
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding_Precision2_OddLastDigit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.2f", 2.375f);  // exactly 2.375 in binary (19/8)
    EXPECT_STREQ(buf.CStr(), "2.38");  // .375 → scaled frac=37 odd, tie rounds up to 38
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding_Precision3_EvenLastDigit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.3f", 2.0625f);  // exactly 2.0625 in binary (33/16)
    EXPECT_STREQ(buf.CStr(), "2.062");  // scaled frac=62 even, tie stays
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding_Precision3_OddLastDigit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.3f", 2.1875f);  // exactly 2.1875 in binary (35/16)
    EXPECT_STREQ(buf.CStr(), "2.188");  // scaled frac=187 odd, tie rounds up to 188
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding_Precision4_EvenLastDigit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.4f", 2.03125f);  // exactly 2.03125 in binary (65/32)
    EXPECT_STREQ(buf.CStr(), "2.0312");  // scaled frac=312 even, tie stays
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding_Precision4_OddLastDigit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.4f", 2.09375f);  // exactly 2.09375 in binary (67/32)
    EXPECT_STREQ(buf.CStr(), "2.0938");  // scaled frac=937 odd, tie rounds up to 938
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding_Precision5_EvenLastDigit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.5f", 2.015625f);  // exactly 2.015625 in binary (129/64)
    EXPECT_STREQ(buf.CStr(), "2.01562");  // scaled frac=1562 even, tie stays
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding_Precision5_OddLastDigit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.5f", 2.046875f);  // exactly 2.046875 in binary (131/64)
    EXPECT_STREQ(buf.CStr(), "2.04688");  // scaled frac=4687 odd, tie rounds up to 4688
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding_Precision6_EvenLastDigit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.6f", 2.0078125f);  // exactly 2.0078125 in binary (257/128)
    EXPECT_STREQ(buf.CStr(), "2.007812");  // scaled frac=7812 even, tie stays
}

TEST(FixedFormatBuffer, FormatFloat_BankersRounding_Precision6_OddLastDigit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.6f", 2.0234375f);  // exactly 2.0234375 in binary (259/128)
    EXPECT_STREQ(buf.CStr(), "2.023438");  // scaled frac=23437 odd, tie rounds up to 23438
}

TEST(FixedFormatBuffer, FormatFloat_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.2f", -1.5f);
    EXPECT_STREQ(buf.CStr(), "-1.50");
}

TEST(FixedFormatBuffer, FormatFloat_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.2f", 0.0f);
    EXPECT_STREQ(buf.CStr(), "0.00");
}

TEST(FixedFormatBuffer, FormatFloat_AlternateForm_ZeroPrecision) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#.0f", 42.0f);
    EXPECT_STREQ(buf.CStr(), "42.");
}

TEST(FixedFormatBuffer, FormatFloat_AlternateForm_ZeroPrecision_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#.0f", -42.0f);
    EXPECT_STREQ(buf.CStr(), "-42.");
}

TEST(FixedFormatBuffer, FormatFloat_AlternateForm_WithPrecision) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#.2f", 3.14f);
    EXPECT_STREQ(buf.CStr(), "3.14");
}

TEST(FixedFormatBuffer, FormatFloat_AlternateForm_Width_RightJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#8.0f", 42.0f);
    EXPECT_STREQ(buf.CStr(), "     42.");
}

TEST(FixedFormatBuffer, FormatFloat_AlternateForm_Width_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#-8.0f", 42.0f);
    EXPECT_STREQ(buf.CStr(), "42.     ");
}

TEST(FixedFormatBuffer, FormatFloat_AlternateForm_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#08.0f", 42.0f);
    EXPECT_STREQ(buf.CStr(), "0000042.");
}

TEST(FixedFormatBuffer, FormatFloat_Nan) {
    FixedFormatBuffer<64> buf;
    buf.Format("%f", std::numeric_limits<float>::quiet_NaN());
    EXPECT_STREQ(buf.CStr(), "nan");
}

TEST(FixedFormatBuffer, FormatFloat_PositiveInf) {
    FixedFormatBuffer<64> buf;
    buf.Format("%f", std::numeric_limits<float>::infinity());
    EXPECT_STREQ(buf.CStr(), "inf");
}

TEST(FixedFormatBuffer, FormatFloat_NegativeInf) {
    FixedFormatBuffer<64> buf;
    buf.Format("%f", -std::numeric_limits<float>::infinity());
    EXPECT_STREQ(buf.CStr(), "-inf");
}

TEST(FixedFormatBuffer, FormatFloat_Overflow) {
    FixedFormatBuffer<64> buf;
    buf.Format("%f", 1e20f);
    EXPECT_STREQ(buf.CStr(), "ovf");
}

TEST(FixedFormatBuffer, FormatFloat_NegativeOverflow) {
    FixedFormatBuffer<64> buf;
    buf.Format("%f", -1e20f);
    EXPECT_STREQ(buf.CStr(), "-ovf");
}

// ---------------------------------------------------------------------------
// Format — mixed args and literal text
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatMixedArgs) {
    FixedFormatBuffer<64> buf;
    buf.Format("x=%i, s=%s", 7, "hi");
    EXPECT_STREQ(buf.CStr(), "x=7, s=hi");
}

TEST(FixedFormatBuffer, FormatLiteralPercent) {
    FixedFormatBuffer<64> buf;
    buf.Format("100%%");
    EXPECT_STREQ(buf.CStr(), "100%");
}

TEST(FixedFormatBuffer, FormatPercentAtEndOfString) {
    FixedFormatBuffer<64> buf;
    buf.Format("done%");
    EXPECT_STREQ(buf.CStr(), "done");
}

TEST(FixedFormatBuffer, FormatUnknownSpecifier) {
    FixedFormatBuffer<64> buf;
    buf.Format("val=%q", 42);
    EXPECT_STREQ(buf.CStr(), "val=%q");
}

// ---------------------------------------------------------------------------
// Format — truncation and return value
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatTruncatesAtCapacity) {
    FixedFormatBuffer<4> buf;
    buf.Format("%s", "hello");
    EXPECT_EQ(buf.Size(), 4u);
    EXPECT_STREQ(buf.CStr(), "hell");
}

TEST(FixedFormatBuffer, FormatReturnsWrittenCount) {
    FixedFormatBuffer<64> buf;
    EXPECT_EQ(buf.Format("%s", "abc"), 3u);
}

TEST(FixedFormatBuffer, SecondFormatOverwritesPrevious) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "first");
    buf.Format("%s", "second");
    EXPECT_STREQ(buf.CStr(), "second");
}

// ---------------------------------------------------------------------------
// Length modifiers
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, LengthMod_hd_Short) {
    FixedFormatBuffer<64> buf;
    buf.Format("%hd", short(42));
    EXPECT_STREQ(buf.CStr(), "42");
}

TEST(FixedFormatBuffer, LengthMod_hd_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%hd", short(-1));
    EXPECT_STREQ(buf.CStr(), "-1");
}

TEST(FixedFormatBuffer, LengthMod_hhd_Char) {
    FixedFormatBuffer<64> buf;
    buf.Format("%hhd", static_cast<signed char>(-128));
    EXPECT_STREQ(buf.CStr(), "-128");
}

TEST(FixedFormatBuffer, LengthMod_ld_Long) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%ld", 42L);
    EXPECT_STREQ(buf.CStr(), "42");
}

TEST(FixedFormatBuffer, LengthMod_lld) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%lld", 42LL);
    EXPECT_STREQ(buf.CStr(), "42");
}

TEST(FixedFormatBuffer, LengthMod_hu) {
    FixedFormatBuffer<64> buf;
    buf.Format("%hu", static_cast<unsigned short>(255));
    EXPECT_STREQ(buf.CStr(), "255");
}

TEST(FixedFormatBuffer, LengthMod_lu) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%lu", 42UL);
    EXPECT_STREQ(buf.CStr(), "42");
}

TEST(FixedFormatBuffer, LengthMod_llu) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%llu", 42ULL);
    EXPECT_STREQ(buf.CStr(), "42");
}

TEST(FixedFormatBuffer, LengthMod_lx) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%lx", 0x2AUL);
    EXPECT_STREQ(buf.CStr(), "2a");
}

TEST(FixedFormatBuffer, LengthMod_llX) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%llX", 0xFFULL);
    EXPECT_STREQ(buf.CStr(), "FF");
}

TEST(FixedFormatBuffer, LengthMod_jd) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%jd", static_cast<intmax_t>(-1));
    EXPECT_STREQ(buf.CStr(), "-1");
}

TEST(FixedFormatBuffer, LengthMod_ju) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%ju", static_cast<uintmax_t>(42));
    EXPECT_STREQ(buf.CStr(), "42");
}

TEST(FixedFormatBuffer, LengthMod_zu) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%zu", static_cast<size_t>(100));
    EXPECT_STREQ(buf.CStr(), "100");
}

TEST(FixedFormatBuffer, LengthMod_td) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%td", static_cast<ptrdiff_t>(-7));
    EXPECT_STREQ(buf.CStr(), "-7");
}

TEST(FixedFormatBuffer, LengthMod_Lf) {
    FixedFormatBuffer<64, LongDoublePolicy> buf;
    buf.Format("%.1Lf", static_cast<long double>(3.1L));
    EXPECT_STREQ(buf.CStr(), "3.1");
}

TEST(FixedFormatBuffer, LengthMod_Combined_FlagsAndLength) {
    FixedFormatBuffer<64, Int64Policy> buf;
    buf.Format("%+#08llx", 0x2AULL);
    EXPECT_STREQ(buf.CStr(), "0x00002a");
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
    EXPECT_STREQ(buf.CStr(), "");
}

// ---------------------------------------------------------------------------
// Copy / assignment
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, CopyConstructor) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "hello");
    FixedFormatBuffer<64> copy{buf};
    EXPECT_STREQ(copy.CStr(), "hello");
    EXPECT_EQ(copy.Size(), 5u);
    // Modifying the original does not affect the copy
    buf.Format("%s", "world");
    EXPECT_STREQ(copy.CStr(), "hello");
}

TEST(FixedFormatBuffer, CopyAssignment) {
    FixedFormatBuffer<64> buf1;
    FixedFormatBuffer<64> buf2;
    buf1.Format("%s", "first");
    buf2.Format("%s", "second");
    buf2 = buf1;
    EXPECT_STREQ(buf2.CStr(), "first");
    // Self-assignment safe
    buf2 = buf2;
    EXPECT_STREQ(buf2.CStr(), "first");
}

// ---------------------------------------------------------------------------
// Format — width (right-align, space-padded)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, Width_Int_WiderThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%6d", 42);
    EXPECT_STREQ(buf.CStr(), "    42");
}

TEST(FixedFormatBuffer, Width_Int_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%2d", 42);
    EXPECT_STREQ(buf.CStr(), "42");
}

TEST(FixedFormatBuffer, Width_Int_NarrowerThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%2d", 12345);
    EXPECT_STREQ(buf.CStr(), "12345"); // no truncation from width
}

TEST(FixedFormatBuffer, Width_Int_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%6d", -7);
    EXPECT_STREQ(buf.CStr(), "    -7");
}

TEST(FixedFormatBuffer, Width_Float_WiderThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%12.2f", 3.14f);
    EXPECT_STREQ(buf.CStr(), "        3.14");
}

TEST(FixedFormatBuffer, Width_Float_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%4.2f", 3.14f);
    EXPECT_STREQ(buf.CStr(), "3.14");
}

TEST(FixedFormatBuffer, Width_Float_NarrowerThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%3.2f", 3.14f);
    EXPECT_STREQ(buf.CStr(), "3.14"); // no truncation from width
}

TEST(FixedFormatBuffer, Width_Float_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%8.2f", -1.5f);
    EXPECT_STREQ(buf.CStr(), "   -1.50");
}

TEST(FixedFormatBuffer, Width_Int_MultiDigitWidth) {
    FixedFormatBuffer<64> buf;
    buf.Format("%10d", 99);
    EXPECT_STREQ(buf.CStr(), "        99");
}

// ---------------------------------------------------------------------------
// Format — 0 flag (zero-pad)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, ZeroPad_Int_WiderThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08d", 42);
    EXPECT_STREQ(buf.CStr(), "00000042");
}

TEST(FixedFormatBuffer, ZeroPad_Int_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%02d", 42);
    EXPECT_STREQ(buf.CStr(), "42");
}

TEST(FixedFormatBuffer, ZeroPad_Int_NarrowerThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%02d", 12345);
    EXPECT_STREQ(buf.CStr(), "12345");
}

TEST(FixedFormatBuffer, ZeroPad_Int_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08d", -7);
    EXPECT_STREQ(buf.CStr(), "-0000007");
}

TEST(FixedFormatBuffer, ZeroPad_Int_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%06d", 0);
    EXPECT_STREQ(buf.CStr(), "000000");
}

TEST(FixedFormatBuffer, ZeroPad_Int_WithShowSign) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+08d", 42);
    EXPECT_STREQ(buf.CStr(), "+0000042");
}

TEST(FixedFormatBuffer, ZeroPad_Int_NegativeWithShowSign) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+08d", -42);
    EXPECT_STREQ(buf.CStr(), "-0000042");
}

TEST(FixedFormatBuffer, ZeroPad_Int_LeftJustifyOverridesZero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-08d", 42);
    EXPECT_STREQ(buf.CStr(), "42      ");
}

TEST(FixedFormatBuffer, ZeroPad_Float_WiderThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08.2f", 3.14f);
    EXPECT_STREQ(buf.CStr(), "00003.14");
}

TEST(FixedFormatBuffer, ZeroPad_Float_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%04.2f", 3.14f);
    EXPECT_STREQ(buf.CStr(), "3.14");
}

TEST(FixedFormatBuffer, ZeroPad_Float_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08.2f", -1.5f);
    EXPECT_STREQ(buf.CStr(), "-0001.50");
}

TEST(FixedFormatBuffer, ZeroPad_Float_WithShowSign) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+08.2f", 1.5f);
    EXPECT_STREQ(buf.CStr(), "+0001.50");
}

TEST(FixedFormatBuffer, ZeroPad_Float_LeftJustifyOverridesZero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-08.2f", 3.14f);
    EXPECT_STREQ(buf.CStr(), "3.14    ");
}

// ---------------------------------------------------------------------------
// Format — + flag (show_sign)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, ShowSign_Int_Positive) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+d", 42);
    EXPECT_STREQ(buf.CStr(), "+42");
}

TEST(FixedFormatBuffer, ShowSign_Int_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+d", -42);
    EXPECT_STREQ(buf.CStr(), "-42");
}

TEST(FixedFormatBuffer, ShowSign_Int_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+d", 0);
    EXPECT_STREQ(buf.CStr(), "+0");
}

TEST(FixedFormatBuffer, ShowSign_Int_WithWidth_RightJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+8d", 42);
    EXPECT_STREQ(buf.CStr(), "     +42");
}

TEST(FixedFormatBuffer, ShowSign_Int_WithWidth_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-+8d", 42);
    EXPECT_STREQ(buf.CStr(), "+42     ");
}

TEST(FixedFormatBuffer, ShowSign_Float_Positive) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+.2f", 1.5f);
    EXPECT_STREQ(buf.CStr(), "+1.50");
}

TEST(FixedFormatBuffer, ShowSign_Float_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+.2f", -1.5f);
    EXPECT_STREQ(buf.CStr(), "-1.50");
}

TEST(FixedFormatBuffer, ShowSign_Float_WithWidth) {
    FixedFormatBuffer<64> buf;
    buf.Format("%+8.2f", 3.14f);
    EXPECT_STREQ(buf.CStr(), "   +3.14");
}

TEST(FixedFormatBuffer, ShowSign_Float_WithWidth_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-+8.2f", 3.14f);
    EXPECT_STREQ(buf.CStr(), "+3.14   ");
}

// ---------------------------------------------------------------------------
// Format — left-justify combined with forced sign (%-+)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, LeftJustifyShowSign_Int_Positive) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-+10d", 42);
    EXPECT_STREQ(buf.CStr(), "+42       ");
}

TEST(FixedFormatBuffer, LeftJustifyShowSign_Int_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-+10d", -42);
    EXPECT_STREQ(buf.CStr(), "-42       ");
}

TEST(FixedFormatBuffer, LeftJustifyShowSign_Int_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-+10d", 0);
    EXPECT_STREQ(buf.CStr(), "+0        ");
}

TEST(FixedFormatBuffer, LeftJustifyShowSign_Float_Positive) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-+12.3f", 3.14f);
    EXPECT_STREQ(buf.CStr(), "+3.140      ");
}

TEST(FixedFormatBuffer, LeftJustifyShowSign_Float_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-+12.3f", -3.14f);
    EXPECT_STREQ(buf.CStr(), "-3.140      ");
}

// ---------------------------------------------------------------------------
// * width and precision
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, StarWidth_Int) {
    FixedFormatBuffer<64> buf;
    buf.Format("%*d", 6, 42);
    EXPECT_STREQ(buf.CStr(), "    42");
}

TEST(FixedFormatBuffer, StarWidth_Negative_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%*d", -6, 42);
    EXPECT_STREQ(buf.CStr(), "42    ");
}

TEST(FixedFormatBuffer, StarPrecision_Float) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.*f", 2, 3.14159f);
    EXPECT_STREQ(buf.CStr(), "3.14");
}

TEST(FixedFormatBuffer, StarWidthAndPrecision_Float) {
    FixedFormatBuffer<64> buf;
    buf.Format("%*.*f", 8, 2, 3.14159f);
    EXPECT_STREQ(buf.CStr(), "    3.14");
}

TEST(FixedFormatBuffer, StarWidthAndPrecision_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-*.*f", 8, 2, 3.14159f);
    EXPECT_STREQ(buf.CStr(), "3.14    ");
}

TEST(FixedFormatBuffer, StarPrecision_Negative_FallsBackToDefault) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.*f", -1, 3.14159f);
    EXPECT_STREQ(buf.CStr(), "3.141590");
}

TEST(FixedFormatBuffer, StarWidth_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%0*d", 6, 42);
    EXPECT_STREQ(buf.CStr(), "000042");
}

// ---------------------------------------------------------------------------
// Space flag (' ')
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, SpaceFlag_Int_Positive) {
    FixedFormatBuffer<64> buf;
    buf.Format("% d", 42);
    EXPECT_STREQ(buf.CStr(), " 42");
}

TEST(FixedFormatBuffer, SpaceFlag_Int_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("% d", -7);
    EXPECT_STREQ(buf.CStr(), "-7");
}

TEST(FixedFormatBuffer, SpaceFlag_Int_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("% d", 0);
    EXPECT_STREQ(buf.CStr(), " 0");
}

TEST(FixedFormatBuffer, SpaceFlag_OverriddenByShowSign_Int) {
    FixedFormatBuffer<64> buf;
    buf.Format("% +d", 42);
    EXPECT_STREQ(buf.CStr(), "+42");
}

TEST(FixedFormatBuffer, SpaceFlag_WithWidth_Int) {
    FixedFormatBuffer<64> buf;
    buf.Format("% 6d", 42);
    EXPECT_STREQ(buf.CStr(), "    42");
}

TEST(FixedFormatBuffer, SpaceFlag_WithZeroPad_Int) {
    FixedFormatBuffer<64> buf;
    buf.Format("% 06d", 42);
    EXPECT_STREQ(buf.CStr(), " 00042");
}

TEST(FixedFormatBuffer, SpaceFlag_Float_Positive) {
    FixedFormatBuffer<64> buf;
    buf.Format("% .1f", 3.1f);
    EXPECT_STREQ(buf.CStr(), " 3.1");
}

TEST(FixedFormatBuffer, SpaceFlag_Float_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("% .1f", -3.1f);
    EXPECT_STREQ(buf.CStr(), "-3.1");
}

TEST(FixedFormatBuffer, SpaceFlag_Float_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("% .1f", 0.0f);
    EXPECT_STREQ(buf.CStr(), " 0.0");
}

TEST(FixedFormatBuffer, SpaceFlag_OverriddenByShowSign_Float) {
    FixedFormatBuffer<64> buf;
    buf.Format("% +.1f", 3.1f);
    EXPECT_STREQ(buf.CStr(), "+3.1");
}

// ---------------------------------------------------------------------------
// Format — %c (character)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatChar_Basic) {
    FixedFormatBuffer<64> buf;
    buf.Format("%c", 'A');
    EXPECT_STREQ(buf.CStr(), "A");
}

TEST(FixedFormatBuffer, FormatChar_Special) {
    FixedFormatBuffer<64> buf;
    buf.Format("%c", '!');
    EXPECT_STREQ(buf.CStr(), "!");
}

TEST(FixedFormatBuffer, FormatChar_InSentence) {
    FixedFormatBuffer<64> buf;
    buf.Format("x=%c y=%c", 'A', 'B');
    EXPECT_STREQ(buf.CStr(), "x=A y=B");
}

TEST(FixedFormatBuffer, FormatChar_Width_RightJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%5c", 'X');
    EXPECT_STREQ(buf.CStr(), "    X");
}

TEST(FixedFormatBuffer, FormatChar_Width_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-5c", 'X');
    EXPECT_STREQ(buf.CStr(), "X    ");
}

TEST(FixedFormatBuffer, FormatChar_Width_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%1c", 'Z');
    EXPECT_STREQ(buf.CStr(), "Z");
}

TEST(FixedFormatBuffer, FormatChar_NegativeChar) {
    FixedFormatBuffer<64> buf;
    buf.Format("%c", 0xFF);
    EXPECT_EQ(buf.Size(), 1u);
    EXPECT_EQ(static_cast<unsigned char>(buf.CStr()[0]), 0xFFu);
}

// ---------------------------------------------------------------------------
// Format — %x (hex)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatHex_Basic) {
    FixedFormatBuffer<64> buf;
    buf.Format("%x", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "2a");
}

TEST(FixedFormatBuffer, FormatHex_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%x", 0U);
    EXPECT_STREQ(buf.CStr(), "0");
}

TEST(FixedFormatBuffer, FormatHex_MaxUint32) {
    FixedFormatBuffer<64> buf;
    buf.Format("%x", 0xFFFFFFFFU);
    EXPECT_STREQ(buf.CStr(), "ffffffff");
}

TEST(FixedFormatBuffer, FormatHex_LowercaseDigits) {
    FixedFormatBuffer<64> buf;
    buf.Format("%x", 0xABCDEFU);
    EXPECT_STREQ(buf.CStr(), "abcdef");
}

TEST(FixedFormatBuffer, FormatHex_Width_RightJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%8x", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "      2a");
}

TEST(FixedFormatBuffer, FormatHex_Width_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-8x", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "2a      ");
}

TEST(FixedFormatBuffer, FormatHex_Width_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%2x", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "2a");
}

TEST(FixedFormatBuffer, FormatHex_Width_NarrowerThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%2x", 0xabcU);
    EXPECT_STREQ(buf.CStr(), "abc");
}

TEST(FixedFormatBuffer, FormatHex_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08x", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "0000002a");
}

TEST(FixedFormatBuffer, FormatHex_ZeroPad_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%02x", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "2a");
}

TEST(FixedFormatBuffer, FormatHex_LeftJustifyOverridesZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-08x", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "2a      ");
}

TEST(FixedFormatBuffer, FormatHex_InSentence) {
    FixedFormatBuffer<64> buf;
    buf.Format("val=0x%x done", 0xffU);
    EXPECT_STREQ(buf.CStr(), "val=0xff done");
}

TEST(FixedFormatBuffer, FormatHex_AlternateForm) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#x", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "0x2a");
}

TEST(FixedFormatBuffer, FormatHex_AlternateForm_ZeroValue) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#x", 0U);
    EXPECT_STREQ(buf.CStr(), "0");
}

TEST(FixedFormatBuffer, FormatHex_AlternateForm_Width_RightJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#8x", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "    0x2a");
}

TEST(FixedFormatBuffer, FormatHex_AlternateForm_Width_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#-8x", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "0x2a    ");
}

TEST(FixedFormatBuffer, FormatHex_AlternateForm_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#08x", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "0x00002a");
}

TEST(FixedFormatBuffer, FormatHex_AlternateForm_ZeroPad_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#06x", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "0x002a");
}

TEST(FixedFormatBuffer, FormatHex_AlternateForm_NarrowerThanContent) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#4x", 0x2afeU);
    EXPECT_STREQ(buf.CStr(), "0x2afe");
}

// ---------------------------------------------------------------------------
// Format — %X (uppercase hex)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatHexUpper_Basic) {
    FixedFormatBuffer<64> buf;
    buf.Format("%X", 0x2afeU);
    EXPECT_STREQ(buf.CStr(), "2AFE");
}

TEST(FixedFormatBuffer, FormatHexUpper_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%X", 0U);
    EXPECT_STREQ(buf.CStr(), "0");
}

TEST(FixedFormatBuffer, FormatHexUpper_Width_RightJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%8X", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "      2A");
}

TEST(FixedFormatBuffer, FormatHexUpper_Width_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-8X", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "2A      ");
}

TEST(FixedFormatBuffer, FormatHexUpper_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08X", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "0000002A");
}

TEST(FixedFormatBuffer, FormatHexUpper_AlternateForm) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#X", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "0X2A");
}

TEST(FixedFormatBuffer, FormatHexUpper_AlternateForm_ZeroValue) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#X", 0U);
    EXPECT_STREQ(buf.CStr(), "0");
}

TEST(FixedFormatBuffer, FormatHexUpper_AlternateForm_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#08X", 0x2aU);
    EXPECT_STREQ(buf.CStr(), "0X00002A");
}

TEST(FixedFormatBuffer, FormatHexUpper_InSentence) {
    FixedFormatBuffer<64> buf;
    buf.Format("val=%X done", 0xffU);
    EXPECT_STREQ(buf.CStr(), "val=FF done");
}

// ---------------------------------------------------------------------------
// Format — %u (unsigned decimal)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, FormatUnsigned_Basic) {
    FixedFormatBuffer<64> buf;
    buf.Format("%u", 42U);
    EXPECT_STREQ(buf.CStr(), "42");
}

TEST(FixedFormatBuffer, FormatUnsigned_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%u", 0U);
    EXPECT_STREQ(buf.CStr(), "0");
}

TEST(FixedFormatBuffer, FormatUnsigned_MaxUint32) {
    FixedFormatBuffer<64> buf;
    buf.Format("%u", 0xFFFFFFFFU);
    EXPECT_STREQ(buf.CStr(), "4294967295");
}

TEST(FixedFormatBuffer, FormatUnsigned_Width_RightJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%8u", 42U);
    EXPECT_STREQ(buf.CStr(), "      42");
}

TEST(FixedFormatBuffer, FormatUnsigned_Width_LeftJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-8u", 42U);
    EXPECT_STREQ(buf.CStr(), "42      ");
}

TEST(FixedFormatBuffer, FormatUnsigned_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08u", 42U);
    EXPECT_STREQ(buf.CStr(), "00000042");
}

TEST(FixedFormatBuffer, FormatUnsigned_ZeroPad_ExactFit) {
    FixedFormatBuffer<64> buf;
    buf.Format("%02u", 42U);
    EXPECT_STREQ(buf.CStr(), "42");
}

TEST(FixedFormatBuffer, FormatUnsigned_LeftJustifyOverridesZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%-08u", 42U);
    EXPECT_STREQ(buf.CStr(), "42      ");
}

// ---------------------------------------------------------------------------
// CStr
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, CStr_ReturnsNullTerminatedString) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "hello");
    const char* s{buf.CStr()};
    EXPECT_STREQ(s, "hello");
}

TEST(FixedFormatBuffer, CStr_EmptyBuffer) {
    FixedFormatBuffer<64> buf;
    const char* s{buf.CStr()};
    EXPECT_STREQ(s, "");
}

TEST(FixedFormatBuffer, CStr_AfterClear) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "data");
    buf.Clear();
    const char* s{buf.CStr()};
    EXPECT_STREQ(s, "");
}

// ---------------------------------------------------------------------------
// Write (const char*)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, Write_CString_Basic) {
    FixedFormatBuffer<64> buf;
    buf.Write("hello");
    EXPECT_STREQ(buf.CStr(), "hello");
    EXPECT_EQ(buf.Size(), 5u);
}

TEST(FixedFormatBuffer, Write_CString_Empty) {
    FixedFormatBuffer<64> buf;
    buf.Write("");
    EXPECT_STREQ(buf.CStr(), "");
    EXPECT_EQ(buf.Size(), 0u);
}

TEST(FixedFormatBuffer, Write_CString_Nullptr) {
    FixedFormatBuffer<64> buf;
    buf.Write(static_cast<const char*>(nullptr));
    EXPECT_STREQ(buf.CStr(), "");
    EXPECT_EQ(buf.Size(), 0u);
}

TEST(FixedFormatBuffer, Write_CString_OverwritesExisting) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "first");
    buf.Write("second");
    EXPECT_STREQ(buf.CStr(), "second");
}

TEST(FixedFormatBuffer, Write_CString_TruncatesAtCapacity) {
    FixedFormatBuffer<4> buf;
    buf.Write("abcdefg");
    EXPECT_STREQ(buf.CStr(), "abcd");
    EXPECT_EQ(buf.Size(), 4u);
}

TEST(FixedFormatBuffer, Write_CString_ReturnsWrittenCount) {
    FixedFormatBuffer<64> buf;
    EXPECT_EQ(buf.Write("abc"), 3u);
}

TEST(FixedFormatBuffer, Write_CString_ExactCapacity) {
    FixedFormatBuffer<5> buf;
    buf.Write("hello");
    EXPECT_STREQ(buf.CStr(), "hello");
    EXPECT_EQ(buf.Size(), 5u);
}

// ---------------------------------------------------------------------------
// Write (const char*, size_t)
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, Write_Len_Basic) {
    FixedFormatBuffer<64> buf;
    buf.Write("abcdef", 3U);
    EXPECT_STREQ(buf.CStr(), "abc");
    EXPECT_EQ(buf.Size(), 3u);
}

TEST(FixedFormatBuffer, Write_Len_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "original");
    buf.Write("abc", 0U);
    EXPECT_STREQ(buf.CStr(), "");
    EXPECT_EQ(buf.Size(), 0u);
}

TEST(FixedFormatBuffer, Write_Len_Nullptr) {
    FixedFormatBuffer<64> buf;
    buf.Write(static_cast<const char*>(nullptr), 3U);
    EXPECT_STREQ(buf.CStr(), "");
    EXPECT_EQ(buf.Size(), 0u);
}

TEST(FixedFormatBuffer, Write_Len_OverwritesExisting) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "first");
    buf.Write("second", 6U);
    EXPECT_STREQ(buf.CStr(), "second");
}

TEST(FixedFormatBuffer, Write_Len_TruncatesAtCapacity) {
    FixedFormatBuffer<4> buf;
    buf.Write("abcdefg", 7U);
    EXPECT_STREQ(buf.CStr(), "abcd");
    EXPECT_EQ(buf.Size(), 4u);
}

TEST(FixedFormatBuffer, Write_Len_ReturnsWrittenCount) {
    FixedFormatBuffer<64> buf;
    EXPECT_EQ(buf.Write("abc", 3U), 3u);
}

TEST(FixedFormatBuffer, Write_Len_LongerThanInput) {
    FixedFormatBuffer<64> buf;
    const char src[]{"hi_____"};
    buf.Write(src, 5U);
    EXPECT_STREQ(buf.CStr(), "hi___");
    EXPECT_EQ(buf.Size(), 5u);
}

// ---------------------------------------------------------------------------
// Append
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, Append_String) {
    FixedFormatBuffer<64> buf;
    buf.Append("%s", "hello");
    buf.Append("%s", " world");
    EXPECT_STREQ(buf.CStr(), "hello world");
}

TEST(FixedFormatBuffer, Append_Int) {
    FixedFormatBuffer<64> buf;
    buf.Append("%d", 10);
    buf.Append("%d", 20);
    EXPECT_STREQ(buf.CStr(), "1020");
}

TEST(FixedFormatBuffer, Append_MixedArgs) {
    FixedFormatBuffer<64> buf;
    buf.Append("x=%d", 1);
    buf.Append(", y=%d", 2);
    EXPECT_STREQ(buf.CStr(), "x=1, y=2");
}

TEST(FixedFormatBuffer, Append_ToEmptyBuffer) {
    FixedFormatBuffer<64> buf;
    buf.Append("%s", "first");
    EXPECT_STREQ(buf.CStr(), "first");
}

TEST(FixedFormatBuffer, Append_TruncatesAtCapacity) {
    FixedFormatBuffer<8> buf;
    buf.Append("%s", "hello");
    buf.Append("%s", ", world!");
    EXPECT_STREQ(buf.CStr(), "hello, w");
    EXPECT_EQ(buf.Size(), 8u);
}

TEST(FixedFormatBuffer, Append_ReturnsSize) {
    FixedFormatBuffer<64> buf;
    buf.Append("%s", "abc");
    EXPECT_EQ(buf.Append("%s", "de"), 5u);
}

TEST(FixedFormatBuffer, Append_PreservesAfterOverflow) {
    FixedFormatBuffer<10> buf;
    buf.Append("%s", "hello");
    buf.Append("%s", " world! extra");  // truncated
    EXPECT_STREQ(buf.CStr(), "hello worl");
    EXPECT_EQ(buf.Size(), 10u);
}

TEST(FixedFormatBuffer, Append_WithLiteralPercent) {
    FixedFormatBuffer<64> buf;
    buf.Append("%d%%", 50);
    EXPECT_STREQ(buf.CStr(), "50%");
}

TEST(FixedFormatBuffer, Append_AfterFormat) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "base");
    buf.Append("+%d", 1);
    EXPECT_STREQ(buf.CStr(), "base+1");
}

TEST(FixedFormatBuffer, Append_Int_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Append("%d", -5);
    buf.Append("%d", -10);
    EXPECT_STREQ(buf.CStr(), "-5-10");
}

TEST(FixedFormatBuffer, Append_Float) {
    FixedFormatBuffer<64> buf;
    buf.Append("%.1f", 1.1f);
    buf.Append("%.1f", 2.2f);
    EXPECT_STREQ(buf.CStr(), "1.12.2");
}

TEST(FixedFormatBuffer, Append_MultipleAppends) {
    FixedFormatBuffer<128> buf;
    buf.Append("%s", "The ");
    buf.Append("%s", "quick ");
    buf.Append("%s", "brown ");
    buf.Append("%s", "fox");
    EXPECT_STREQ(buf.CStr(), "The quick brown fox");
}

// ---------------------------------------------------------------------------
// operator== / operator!=
// ---------------------------------------------------------------------------

TEST(FixedFormatBuffer, OperatorEquals_BufferVsBuffer_Same) {
    FixedFormatBuffer<64> a;
    FixedFormatBuffer<64> b;
    a.Format("%s", "hello");
    b.Format("%s", "hello");
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(FixedFormatBuffer, OperatorEquals_BufferVsBuffer_Different) {
    FixedFormatBuffer<64> a;
    FixedFormatBuffer<64> b;
    a.Format("%s", "hello");
    b.Format("%s", "world");
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST(FixedFormatBuffer, OperatorEquals_BufferVsBuffer_Empty) {
    FixedFormatBuffer<64> a;
    FixedFormatBuffer<64> b;
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(FixedFormatBuffer, OperatorEquals_BufferVsStringView_Same) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "hello");
    EXPECT_TRUE(buf == std::string_view{"hello"});
    EXPECT_FALSE(buf != std::string_view{"hello"});
}

TEST(FixedFormatBuffer, OperatorEquals_BufferVsStringView_Different) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "hello");
    EXPECT_FALSE(buf == std::string_view{"world"});
    EXPECT_TRUE(buf != std::string_view{"world"});
}

TEST(FixedFormatBuffer, OperatorEquals_StringViewVsBuffer_Same) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "hello");
    EXPECT_TRUE(std::string_view{"hello"} == buf);
    EXPECT_FALSE(std::string_view{"hello"} != buf);
}

TEST(FixedFormatBuffer, OperatorEquals_StringViewVsBuffer_Different) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "hello");
    EXPECT_FALSE(std::string_view{"world"} == buf);
    EXPECT_TRUE(std::string_view{"world"} != buf);
}

TEST(FixedFormatBuffer, OperatorEquals_DifferentPositions) {
    FixedFormatBuffer<64> a;
    FixedFormatBuffer<64> b;
    a.Append("%s", "ab");
    a.Append("%s", "cd");
    b.Format("%s", "abcd");
    EXPECT_TRUE(a == b);
}
