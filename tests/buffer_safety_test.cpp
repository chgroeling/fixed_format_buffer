#include "ffb/fixed_format_buffer.h"

#include <climits>
#include <cstring>
#include <string_view>

#include <gtest/gtest.h>

using ffb::FixedFormatBuffer;

// ---------------------------------------------------------------------------
// Helper: a FixedFormatBuffer wrapped between two sentinel regions.
//
// Layout (no padding inserted by the compiler between members because
// FixedFormatBuffer contains only char[] and size_t, both naturally
// aligned; the sentinel arrays are char so they have alignment 1):
//
//   [ before_sentinel | FixedFormatBuffer<N> | after_sentinel ]
//
// Any write that escapes the buffer_ array and crosses the object
// boundary will corrupt one of the sentinels and be caught by
// CheckSentinels().
// ---------------------------------------------------------------------------
template <std::size_t N>
struct Guarded {
    static constexpr uint8_t kFill = 0xCC;

    uint8_t              before[16];
    FixedFormatBuffer<N> buf;
    uint8_t              after[16];

    Guarded() {
        std::memset(before, kFill, sizeof(before));
        std::memset(after,  kFill, sizeof(after));
    }

    void CheckSentinels() const {
        for (std::size_t i = 0U; i < sizeof(before); ++i)
            EXPECT_EQ(before[i], kFill) << "before-sentinel corrupted at index " << i;
        for (std::size_t i = 0U; i < sizeof(after); ++i)
            EXPECT_EQ(after[i],  kFill) << "after-sentinel corrupted at index " << i;
    }
};

// ---------------------------------------------------------------------------
// Observable safety invariants after an overflowing Format call
// ---------------------------------------------------------------------------

TEST(BufferSafety, SizeNeverExceedsCapacity) {
    FixedFormatBuffer<4> buf;
    buf.Format("%s", "hello world");  // 11 chars > capacity 4
    EXPECT_LE(buf.Size(), buf.CAPACITY);
}

TEST(BufferSafety, ViewSizeMatchesSize) {
    FixedFormatBuffer<4> buf;
    buf.Format("%s", "hello");
    EXPECT_EQ(buf.View().size(), buf.Size());
}

TEST(BufferSafety, NullTerminatorAlwaysPresent) {
    // buffer_ has N+1 chars; buffer_[Size()] must always be '\0'.
    FixedFormatBuffer<4> buf;
    buf.Format("%s", "hello world");
    EXPECT_EQ(buf.View().data()[buf.Size()], '\0');
}

TEST(BufferSafety, ContentIsCorrectPrefix) {
    FixedFormatBuffer<4> buf;
    buf.Format("%s", "hello");
    EXPECT_EQ(buf.View(), "hell");
    EXPECT_EQ(buf.Size(), 4u);
}

TEST(BufferSafety, NullTerminatorPresentAfterExactFit) {
    FixedFormatBuffer<5> buf;
    buf.Format("%s", "hello");  // exactly 5 chars = capacity
    EXPECT_EQ(buf.View(), "hello");
    EXPECT_EQ(buf.View().data()[buf.Size()], '\0');
}

TEST(BufferSafety, NullTerminatorPresentAfterShortWrite) {
    FixedFormatBuffer<64> buf;
    buf.Format("%s", "hi");
    EXPECT_EQ(buf.View().data()[buf.Size()], '\0');
}

// ---------------------------------------------------------------------------
// Integer overflow scenarios
// ---------------------------------------------------------------------------

TEST(BufferSafety, IntOverflow_SizeInvariant) {
    FixedFormatBuffer<3> buf;
    buf.Format("%i", -2147483648);  // "-2147483648" = 11 chars > 3
    EXPECT_LE(buf.Size(), buf.CAPACITY);
    EXPECT_EQ(buf.View().data()[buf.Size()], '\0');
}

// ---------------------------------------------------------------------------
// Float overflow scenarios
// ---------------------------------------------------------------------------

TEST(BufferSafety, FloatOverflow_SizeInvariant) {
    FixedFormatBuffer<3> buf;
    buf.Format("%f", 3.14f);  // "3.140000" = 8 chars > 3
    EXPECT_LE(buf.Size(), buf.CAPACITY);
    EXPECT_EQ(buf.View().data()[buf.Size()], '\0');
}

// ---------------------------------------------------------------------------
// Sentinel tests — detect writes past the FixedFormatBuffer object boundary
// ---------------------------------------------------------------------------

TEST(BufferSafety, Sentinel_StringOverflow) {
    Guarded<4> g;
    g.buf.Format("%s", "hello world");
    g.CheckSentinels();
    EXPECT_LE(g.buf.Size(), 4u);
}

TEST(BufferSafety, Sentinel_IntOverflow) {
    Guarded<3> g;
    g.buf.Format("%i", -2147483648);
    g.CheckSentinels();
    EXPECT_LE(g.buf.Size(), 3u);
}

TEST(BufferSafety, Sentinel_FloatOverflow) {
    Guarded<3> g;
    g.buf.Format("%f", 3.14f);
    g.CheckSentinels();
    EXPECT_LE(g.buf.Size(), 3u);
}

TEST(BufferSafety, Sentinel_MultipleFormats) {
    Guarded<4> g;
    for (int i = 0; i < 100; ++i) {
        g.buf.Format("%i", i * 1000);
    }
    g.CheckSentinels();
    EXPECT_LE(g.buf.Size(), 4u);
}

// ---------------------------------------------------------------------------
// Null terminator placement on reuse (second Format over previous content)
// ---------------------------------------------------------------------------

// Long → short: null terminator must move to the new (shorter) position.
TEST(BufferSafety, Reuse_LongToShort_NullTerminatorCorrect) {
    FixedFormatBuffer<16> buf;
    buf.Format("%s", "hello");            // size=5, buffer[5]='\0'
    buf.Format("%s", "hi");               // size=2, buffer[2] must be '\0'

    EXPECT_EQ(buf.Size(), 2u);
    EXPECT_EQ(buf.View(), "hi");
    EXPECT_EQ(buf.View().data()[buf.Size()], '\0');
}

// Short → long (fits): null terminator moves forward.
TEST(BufferSafety, Reuse_ShortToLong_NullTerminatorCorrect) {
    FixedFormatBuffer<16> buf;
    buf.Format("%s", "hi");               // size=2, buffer[2]='\0'
    buf.Format("%s", "hello");            // size=5, buffer[5] must be '\0'

    EXPECT_EQ(buf.Size(), 5u);
    EXPECT_EQ(buf.View(), "hello");
    EXPECT_EQ(buf.View().data()[buf.Size()], '\0');
}

// Long → truncated: null terminator lands exactly at CAPACITY.
TEST(BufferSafety, Reuse_LongToTruncated_NullTerminatorAtCapacity) {
    FixedFormatBuffer<4> buf;
    buf.Format("%s", "hi");               // size=2, buffer[2]='\0'
    buf.Format("%s", "hello world");      // truncated to 4, buffer[4] must be '\0'

    EXPECT_EQ(buf.Size(), 4u);
    EXPECT_EQ(buf.View(), "hell");
    EXPECT_EQ(buf.View().data()[buf.Size()], '\0');
}

// Any → empty: null terminator must be at position 0.
TEST(BufferSafety, Reuse_ToEmpty_NullTerminatorAtZero) {
    FixedFormatBuffer<16> buf;
    buf.Format("%s", "hello");
    buf.Format("%s", "");                 // size=0, buffer[0] must be '\0'

    EXPECT_EQ(buf.Size(), 0u);
    EXPECT_TRUE(buf.Empty());
    EXPECT_EQ(buf.View().data()[0], '\0');
}

// ---------------------------------------------------------------------------
// Null terminator at exactly position N (buffer filled to capacity)
//
// buffer_ has N+1 slots; when exactly N chars are written, g.pos == N and
// the terminator goes into buffer_[N] — the last (reserved) slot.
// ---------------------------------------------------------------------------

TEST(BufferSafety, ExactFill_String_NullTerminatorAtN) {
    FixedFormatBuffer<5> buf;
    buf.Format("%s", "hello");            // exactly 5 chars = N
    EXPECT_EQ(buf.Size(), 5u);
    EXPECT_EQ(buf.View(), "hello");
    EXPECT_EQ(buf.View().data()[5], '\0'); // buffer_[N] must be '\0'
}

TEST(BufferSafety, ExactFill_Int_NullTerminatorAtN) {
    FixedFormatBuffer<3> buf;
    buf.Format("%i", 42);                 // "42" = 2 chars < 3; use 999 → 3 chars
    // First verify 2-char case (sanity)
    EXPECT_EQ(buf.View().data()[buf.Size()], '\0');

    buf.Format("%i", 999);               // exactly 3 chars = N
    EXPECT_EQ(buf.Size(), 3u);
    EXPECT_EQ(buf.View(), "999");
    EXPECT_EQ(buf.View().data()[3], '\0');
}

TEST(BufferSafety, ExactFill_Float_NullTerminatorAtN) {
    // "1.50" = 4 chars; use FixedFormatBuffer<4>
    FixedFormatBuffer<4> buf;
    buf.Format("%.2f", 1.5f);            // exactly 4 chars = N
    EXPECT_EQ(buf.Size(), 4u);
    EXPECT_EQ(buf.View(), "1.50");
    EXPECT_EQ(buf.View().data()[4], '\0');
}

TEST(BufferSafety, ExactFill_AfterShorterWrite_NullTerminatorAtN) {
    FixedFormatBuffer<5> buf;
    buf.Format("%s", "hi");              // size=2, buffer[2]='\0'
    buf.Format("%s", "hello");           // size=5=N, buffer[5] must be '\0'
    EXPECT_EQ(buf.Size(), 5u);
    EXPECT_EQ(buf.View(), "hello");
    EXPECT_EQ(buf.View().data()[5], '\0');
}

// ---------------------------------------------------------------------------
// Integer longer than buffer — truncation and null terminator at buffer_[N]
// ---------------------------------------------------------------------------

TEST(BufferSafety, IntLongerThanBuffer_PositiveOverflow) {
    FixedFormatBuffer<3> buf;
    buf.Format("%i", 12345);     // "12345" = 5 chars > 3
    EXPECT_EQ(buf.Size(), 3u);
    EXPECT_EQ(buf.View(), "123");
    EXPECT_EQ(buf.View().data()[3], '\0');
}

TEST(BufferSafety, IntLongerThanBuffer_NegativeOverflow) {
    FixedFormatBuffer<3> buf;
    buf.Format("%i", -1234);     // "-1234" = 5 chars > 3
    EXPECT_EQ(buf.Size(), 3u);
    EXPECT_EQ(buf.View(), "-12");
    EXPECT_EQ(buf.View().data()[3], '\0');
}

TEST(BufferSafety, IntLongerThanBuffer_OnlySignFits) {
    FixedFormatBuffer<1> buf;
    buf.Format("%i", -99);       // "-99" = 3 chars; only '-' fits
    EXPECT_EQ(buf.Size(), 1u);
    EXPECT_EQ(buf.View(), "-");
    EXPECT_EQ(buf.View().data()[1], '\0');
}

TEST(BufferSafety, IntLongerThanBuffer_NothingFits) {
    FixedFormatBuffer<0> buf;
    buf.Format("%i", 42);        // no room at all; buffer_[0] must be '\0'
    EXPECT_EQ(buf.Size(), 0u);
    EXPECT_EQ(buf.View(), "");
    EXPECT_EQ(buf.View().data()[0], '\0');
}

// ---------------------------------------------------------------------------
// Float longer than buffer — truncation and null terminator at buffer_[N]
// ---------------------------------------------------------------------------

TEST(BufferSafety, FloatLongerThanBuffer_TruncatesDigits) {
    FixedFormatBuffer<3> buf;
    buf.Format("%.2f", 3.14f);   // "3.14" = 4 chars > 3
    EXPECT_EQ(buf.Size(), 3u);
    EXPECT_EQ(buf.View(), "3.1");
    EXPECT_EQ(buf.View().data()[3], '\0');
}

TEST(BufferSafety, FloatLongerThanBuffer_TruncatesAfterDecimalPoint) {
    FixedFormatBuffer<2> buf;
    buf.Format("%.2f", 3.14f);   // "3.14" = 4 chars; only "3." fits
    EXPECT_EQ(buf.Size(), 2u);
    EXPECT_EQ(buf.View(), "3.");
    EXPECT_EQ(buf.View().data()[2], '\0');
}

TEST(BufferSafety, FloatLongerThanBuffer_TruncatesBeforeDecimalPoint) {
    FixedFormatBuffer<1> buf;
    buf.Format("%.2f", 3.14f);   // only "3" fits
    EXPECT_EQ(buf.Size(), 1u);
    EXPECT_EQ(buf.View(), "3");
    EXPECT_EQ(buf.View().data()[1], '\0');
}

TEST(BufferSafety, FloatLongerThanBuffer_NegativeSign) {
    FixedFormatBuffer<3> buf;
    buf.Format("%.2f", -3.14f);  // "-3.14" = 5 chars > 3
    EXPECT_EQ(buf.Size(), 3u);
    EXPECT_EQ(buf.View(), "-3.");
    EXPECT_EQ(buf.View().data()[3], '\0');
}

TEST(BufferSafety, IntLongerThanBuffer_OnlySignSurvives) {
    FixedFormatBuffer<1> buf;
    buf.Format("%i", -42);       // "-42" = 3 chars; only '-' fits in N=1
    EXPECT_EQ(buf.Size(), 1u);
    EXPECT_EQ(buf.View(), "-");
    EXPECT_EQ(buf.View().data()[1], '\0');
}

TEST(BufferSafety, FloatLongerThanBuffer_OnlySignSurvives) {
    FixedFormatBuffer<1> buf;
    buf.Format("%.2f", -3.14f);  // "-3.14" = 5 chars; only '-' fits in N=1
    EXPECT_EQ(buf.Size(), 1u);
    EXPECT_EQ(buf.View(), "-");
    EXPECT_EQ(buf.View().data()[1], '\0');
}
