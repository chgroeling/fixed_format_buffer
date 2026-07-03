#include "ffb/fixed_format_buffer.h"

#include <cstdarg>
#include <cstdio>
#include <string>

#include <gtest/gtest.h>

using ffb::FixedFormatBuffer;

// ---------------------------------------------------------------------------
// Helper: produce the sprintf reference output in a std::string.
// ---------------------------------------------------------------------------

static std::string Ref(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return buf;
}

// ---------------------------------------------------------------------------
// %s — string
// ---------------------------------------------------------------------------

TEST(SprintfComparison, String_Basic) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "hello");
    EXPECT_EQ(buf.View(), Ref("%s", "hello"));
}

TEST(SprintfComparison, String_Empty) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "");
    EXPECT_EQ(buf.View(), Ref("%s", ""));
}

TEST(SprintfComparison, String_Embedded_In_Text) {
    FixedFormatBuffer<64> buf;
    buf.Format("key=%s!", "val");
    EXPECT_EQ(buf.View(), Ref("key=%s!", "val"));
}

// ---------------------------------------------------------------------------
// %i / %d — signed integer
// ---------------------------------------------------------------------------

TEST(SprintfComparison, Int_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%i", 0);
    EXPECT_EQ(buf.View(), Ref("%i", 0));
}

TEST(SprintfComparison, Int_Positive) {
    FixedFormatBuffer<64> buf;
    buf.Format("%i", 12345);
    EXPECT_EQ(buf.View(), Ref("%i", 12345));
}

TEST(SprintfComparison, Int_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%i", -12345);
    EXPECT_EQ(buf.View(), Ref("%i", -12345));
}

TEST(SprintfComparison, Int_Max) {
    FixedFormatBuffer<64> buf;
    buf.Format("%i", 2147483647);
    EXPECT_EQ(buf.View(), Ref("%i", 2147483647));
}

TEST(SprintfComparison, Int_Min) {
    FixedFormatBuffer<64> buf;
    buf.Format("%i", -2147483648);
    EXPECT_EQ(buf.View(), Ref("%i", -2147483648));
}

TEST(SprintfComparison, Int_Specifier_d) {
    FixedFormatBuffer<64> buf;
    buf.Format("%d", 99);
    EXPECT_EQ(buf.View(), Ref("%d", 99));
}

TEST(SprintfComparison, Int_In_Sentence) {
    FixedFormatBuffer<64> buf;
    buf.Format("count=%i items", 7);
    EXPECT_EQ(buf.View(), Ref("count=%i items", 7));
}

// ---------------------------------------------------------------------------
// %f — float
//
// Note: FixedFormatBuffer uses float (single precision) internally.
// Test values are chosen so that the float and double representations
// agree after rounding to the given precision.
//
// Note: FixedFormatBuffer applies banker's rounding (round-half-to-even),
// while sprintf typically uses round-half-away-from-zero. Values exactly
// halfway between two representations (e.g. 2.5 at precision 0) are
// therefore intentionally excluded from this comparison suite.
// ---------------------------------------------------------------------------

TEST(SprintfComparison, Float_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%f", 0.0f);
    EXPECT_EQ(buf.View(), Ref("%f", 0.0f));
}

TEST(SprintfComparison, Float_One) {
    FixedFormatBuffer<64> buf;
    buf.Format("%f", 1.0f);
    EXPECT_EQ(buf.View(), Ref("%f", 1.0f));
}

TEST(SprintfComparison, Float_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%f", -1.0f);
    EXPECT_EQ(buf.View(), Ref("%f", -1.0f));
}

TEST(SprintfComparison, Float_Precision2) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.2f", 3.14f);
    EXPECT_EQ(buf.View(), Ref("%.2f", 3.14f));
}

TEST(SprintfComparison, Float_Precision2_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.2f", -1.75f);  // -1.75 is exact in binary
    EXPECT_EQ(buf.View(), Ref("%.2f", -1.75f));
}

TEST(SprintfComparison, Float_Precision0_NoHalfway) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.0f", 2.7f);
    EXPECT_EQ(buf.View(), Ref("%.0f", 2.7f));
}

TEST(SprintfComparison, Float_Precision4_ExactBinary) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.4f", 1.0625f);  // 1 + 1/16, exact in binary
    EXPECT_EQ(buf.View(), Ref("%.4f", 1.0625f));
}

TEST(SprintfComparison, Float_Large_Integral) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.2f", 1024.25f);  // exact in binary
    EXPECT_EQ(buf.View(), Ref("%.2f", 1024.25f));
}

TEST(SprintfComparison, Float_AlternateForm_ZeroPrecision) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#.0f", 42.0f);
    EXPECT_EQ(buf.View(), Ref("%#.0f", 42.0f));
}

TEST(SprintfComparison, Float_AlternateForm_WithPrecision) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#.2f", 3.14f);
    EXPECT_EQ(buf.View(), Ref("%#.2f", 3.14f));
}

TEST(SprintfComparison, Float_AlternateForm_Width_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#08.0f", 42.0f);
    EXPECT_EQ(buf.View(), Ref("%#08.0f", 42.0f));
}

// ---------------------------------------------------------------------------
// Mixed format strings
// ---------------------------------------------------------------------------

TEST(SprintfComparison, Mixed_IntAndString) {
    FixedFormatBuffer<64> buf;
    buf.Format("i=%i s=%s", 42, "hi");
    EXPECT_EQ(buf.View(), Ref("i=%i s=%s", 42, "hi"));
}

TEST(SprintfComparison, Mixed_StringAndFloat) {
    FixedFormatBuffer<64> buf;
    buf.Format("v=%.2f u=%s", 3.14f, "cm");
    EXPECT_EQ(buf.View(), Ref("v=%.2f u=%s", 3.14f, "cm"));
}

TEST(SprintfComparison, LiteralPercent) {
    FixedFormatBuffer<64> buf;
    buf.Format("100%%");
    EXPECT_EQ(buf.View(), Ref("100%%"));
}

// ---------------------------------------------------------------------------
// %x — hex
// ---------------------------------------------------------------------------

TEST(SprintfComparison, Hex_Basic) {
    FixedFormatBuffer<64> buf;
    buf.Format("%x", 42U);
    EXPECT_EQ(buf.View(), Ref("%x", 42U));
}

TEST(SprintfComparison, Hex_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%x", 0U);
    EXPECT_EQ(buf.View(), Ref("%x", 0U));
}

TEST(SprintfComparison, Hex_Large) {
    FixedFormatBuffer<64> buf;
    buf.Format("%x", 0x7FFFFFFFU);
    EXPECT_EQ(buf.View(), Ref("%x", 0x7FFFFFFFU));
}

TEST(SprintfComparison, Hex_Width_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08x", 0xffU);
    EXPECT_EQ(buf.View(), Ref("%08x", 0xffU));
}

TEST(SprintfComparison, Hex_AlternateForm) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#x", 0x2aU);
    EXPECT_EQ(buf.View(), Ref("%#x", 0x2aU));
}

TEST(SprintfComparison, Hex_AlternateForm_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#x", 0U);
    EXPECT_EQ(buf.View(), Ref("%#x", 0U));
}

TEST(SprintfComparison, Hex_AlternateForm_Width_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#08x", 0x2aU);
    EXPECT_EQ(buf.View(), Ref("%#08x", 0x2aU));
}

TEST(SprintfComparison, Hex_AlternateForm_Width_RightJustify) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#8x", 0x2aU);
    EXPECT_EQ(buf.View(), Ref("%#8x", 0x2aU));
}

TEST(SprintfComparison, HexUpper_Basic) {
    FixedFormatBuffer<64> buf;
    buf.Format("%X", 0x2afeU);
    EXPECT_EQ(buf.View(), Ref("%X", 0x2afeU));
}

TEST(SprintfComparison, HexUpper_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%X", 0U);
    EXPECT_EQ(buf.View(), Ref("%X", 0U));
}

TEST(SprintfComparison, HexUpper_Width_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%08X", 0x2aU);
    EXPECT_EQ(buf.View(), Ref("%08X", 0x2aU));
}

TEST(SprintfComparison, HexUpper_AlternateForm) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#X", 0x2aU);
    EXPECT_EQ(buf.View(), Ref("%#X", 0x2aU));
}

TEST(SprintfComparison, HexUpper_AlternateForm_ZeroPad) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#08X", 0x2aU);
    EXPECT_EQ(buf.View(), Ref("%#08X", 0x2aU));
}

// ---------------------------------------------------------------------------
// Length modifiers
// ---------------------------------------------------------------------------

TEST(SprintfComparison, LengthMod_hd) {
    FixedFormatBuffer<64> buf;
    buf.Format("%hd", short(-1));
    EXPECT_EQ(buf.View(), Ref("%hd", short(-1)));
}

TEST(SprintfComparison, LengthMod_hhd) {
    FixedFormatBuffer<64> buf;
    buf.Format("%hhd", static_cast<signed char>(-128));
    EXPECT_EQ(buf.View(), Ref("%hhd", static_cast<signed char>(-128)));
}

TEST(SprintfComparison, LengthMod_ld) {
    FixedFormatBuffer<64> buf;
    buf.Format("%ld", 123456L);
    EXPECT_EQ(buf.View(), Ref("%ld", 123456L));
}

TEST(SprintfComparison, LengthMod_lld) {
    FixedFormatBuffer<64> buf;
    buf.Format("%lld", 42LL);
    EXPECT_EQ(buf.View(), Ref("%lld", 42LL));
}

TEST(SprintfComparison, LengthMod_hu) {
    FixedFormatBuffer<64> buf;
    buf.Format("%hu", static_cast<unsigned short>(42));
    EXPECT_EQ(buf.View(), Ref("%hu", static_cast<unsigned short>(42)));
}

TEST(SprintfComparison, LengthMod_lu) {
    FixedFormatBuffer<64> buf;
    buf.Format("%lu", 42UL);
    EXPECT_EQ(buf.View(), Ref("%lu", 42UL));
}

TEST(SprintfComparison, LengthMod_llu) {
    FixedFormatBuffer<64> buf;
    buf.Format("%llu", 12345ULL);
    EXPECT_EQ(buf.View(), Ref("%llu", 12345ULL));
}

TEST(SprintfComparison, LengthMod_lx) {
    FixedFormatBuffer<64> buf;
    buf.Format("%lx", 0xABCDUL);
    EXPECT_EQ(buf.View(), Ref("%lx", 0xABCDUL));
}

TEST(SprintfComparison, LengthMod_llX) {
    FixedFormatBuffer<64> buf;
    buf.Format("%#llX", 0xFFULL);
    EXPECT_EQ(buf.View(), Ref("%#llX", 0xFFULL));
}

TEST(SprintfComparison, LengthMod_zu) {
    FixedFormatBuffer<64> buf;
    buf.Format("%zu", static_cast<size_t>(100));
    EXPECT_EQ(buf.View(), Ref("%zu", static_cast<size_t>(100)));
}

TEST(SprintfComparison, LengthMod_td) {
    FixedFormatBuffer<64> buf;
    buf.Format("%td", static_cast<ptrdiff_t>(-7));
    EXPECT_EQ(buf.View(), Ref("%td", static_cast<ptrdiff_t>(-7)));
}

TEST(SprintfComparison, LengthMod_jd) {
    FixedFormatBuffer<64> buf;
    buf.Format("%jd", static_cast<intmax_t>(-42));
    EXPECT_EQ(buf.View(), Ref("%jd", static_cast<intmax_t>(-42)));
}

TEST(SprintfComparison, LengthMod_Lf) {
    FixedFormatBuffer<64> buf;
    buf.Format("%.1Lf", static_cast<long double>(3.1L));
    EXPECT_EQ(buf.View(), Ref("%.1Lf", static_cast<long double>(3.1L)));
}

// ---------------------------------------------------------------------------
// %u — unsigned decimal
// ---------------------------------------------------------------------------

TEST(SprintfComparison, Unsigned_Basic) {
    FixedFormatBuffer<64> buf;
    buf.Format("%u", 42U);
    EXPECT_EQ(buf.View(), Ref("%u", 42U));
}

TEST(SprintfComparison, Unsigned_Zero) {
    FixedFormatBuffer<64> buf;
    buf.Format("%u", 0U);
    EXPECT_EQ(buf.View(), Ref("%u", 0U));
}

TEST(SprintfComparison, Unsigned_Large) {
    FixedFormatBuffer<64> buf;
    buf.Format("%u", 0x7FFFFFFFU);
    EXPECT_EQ(buf.View(), Ref("%u", 0x7FFFFFFFU));
}

// ---------------------------------------------------------------------------
// Space flag
// ---------------------------------------------------------------------------

TEST(SprintfComparison, SpaceFlag_Int) {
    FixedFormatBuffer<64> buf;
    buf.Format("% d", 42);
    EXPECT_EQ(buf.View(), Ref("% d", 42));
}

TEST(SprintfComparison, SpaceFlag_Int_Negative) {
    FixedFormatBuffer<64> buf;
    buf.Format("% d", -7);
    EXPECT_EQ(buf.View(), Ref("% d", -7));
}

TEST(SprintfComparison, SpaceFlag_Float) {
    FixedFormatBuffer<64> buf;
    buf.Format("% .1f", 3.1f);
    EXPECT_EQ(buf.View(), Ref("% .1f", 3.1f));
}

TEST(SprintfComparison, SpaceFlag_OverriddenByShowSign) {
    FixedFormatBuffer<64> buf;
    buf.Format("% +d", 42);
    EXPECT_EQ(buf.View(), Ref("% +d", 42));
}

TEST(SprintfComparison, SpaceFlag_ZeroPad_Int) {
    FixedFormatBuffer<64> buf;
    buf.Format("% 06d", 42);
    EXPECT_EQ(buf.View(), Ref("% 06d", 42));
}
