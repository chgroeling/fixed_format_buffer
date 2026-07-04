#include "ffb/fixed_format_buffer.h"

#include <cstdlib>
#include <string>

static int failures{0};

static void check(bool cond, const char* msg) {
    if (!cond) {
        std::printf("FAIL: %s\n", msg);
        ++failures;
    }
}

int main() {
    // --- basic formatting ---
    ffb::FixedFormatBuffer<64> buf;
    buf.Format("%s", "hello");
    check(std::string{buf.CStr()} == "hello", "string format");

    buf.Format("%d", 42);
    check(std::string{buf.CStr()} == "42", "int format");

    buf.Format("%.2f", 3.14f);
    check(std::string{buf.CStr()} == "3.14", "float format");

    // --- append ---
    buf.Format("foo");
    buf.Append("=%d", 1);
    check(std::string{buf.CStr()} == "foo=1", "append");

    // --- write ---
    buf.Write("raw");
    check(std::string{buf.CStr()} == "raw", "write");

    // --- width + sign ---
    buf.Format("%+08d", 42);
    check(std::string{buf.CStr()} == "+0000042", "zero-pad + sign");

    // --- hex ---
    buf.Format("%#x", 0x2AU);
    check(std::string{buf.CStr()} == "0x2a", "hex alternate");

    // --- truncation ---
    ffb::FixedFormatBuffer<4> small;
    small.Format("%s", "hello world");
    check(small.Size() == 4U, "truncation size");
    check(std::string{small.CStr()} == "hell", "truncation content");

    // --- Int64Policy ---
    ffb::FixedFormatBuffer<64, ffb::Int64Policy> big;
    big.Format("%llu", UINT64_C(18446744073709551615));
    check(std::string{big.CStr()} == "18446744073709551615", "uint64 max");

    if (failures) {
        std::printf("\n%d test(s) FAILED\n", failures);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
