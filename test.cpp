// test_FixedString.cpp
//
// Unit tests for FixedString<N> using doctest (https://github.com/doctest/doctest).
//
// Build (single header doctest.h alongside this file):
//   g++ -std=c++17 -Wall -Wextra test_FixedString.cpp -o test_FixedString
//   ./test_FixedString
//

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "FixedString.h"

TEST_CASE("construction: fits comfortably within capacity") {
    FixedString<32> s("Hello, World!");
    CHECK(s.size() == 13);
    CHECK(s.capacity() == 31);              // capacity excludes the reserved null terminator byte
    CHECK(std::string(s.c_str()) == "Hello, World!");
    CHECK_FALSE(s.is_truncated());
}

TEST_CASE("construction: content exactly fills usable capacity") {
    // capacity<5>() == 4 usable chars + 1 reserved for '\0'
    FixedString<5> s("abcd");
    CHECK(s.size() == 4);
    CHECK(s.capacity() == 4);
    CHECK(std::string(s.c_str()) == "abcd");
    CHECK_FALSE(s.is_truncated());
}

TEST_CASE("construction: content longer than capacity truncates and sets the flag") {
    FixedString<5> s("abcdefgh"); // usable capacity is 4
    CHECK(s.size() == 4);
    CHECK(std::string(s.c_str()) == "abcd");
    CHECK(s.is_truncated());
}

TEST_CASE("construction: nullptr content yields an empty, non-truncated string") {
    FixedString<32> s(nullptr);
    CHECK(s.size() == 0);
    CHECK(std::string(s.c_str()) == "");
    CHECK_FALSE(s.is_truncated());
}

TEST_CASE("construction: empty string literal") {
    FixedString<32> s("");
    CHECK(s.size() == 0);
    CHECK(std::string(s.c_str()) == "");
    CHECK_FALSE(s.is_truncated());
}

TEST_CASE("append: grows an existing string within capacity") {
    FixedString<32> s("Hello");
    int rc = s.append(", World!");
    CHECK(rc == 0);
    CHECK(s.size() == 13);
    CHECK(std::string(s.c_str()) == "Hello, World!");
    CHECK_FALSE(s.is_truncated());
}

TEST_CASE("append: content overflowing remaining capacity truncates and sets the flag") {
    FixedString<10> s("1234"); // 6 usable chars remain (capacity 9, 4 used)
    int rc = s.append("567890"); // only "56789" fits before the reserved null byte
    CHECK(rc == -1);
    CHECK(s.is_truncated());
    CHECK(std::string(s.c_str()) == "123456789");
    CHECK(s.size() == 9);
}

TEST_CASE("append: nullptr content is rejected without modifying the string") {
    FixedString<32> s("Hello");
    int rc = s.append(nullptr);
    CHECK(rc == -1);
    CHECK(s.size() == 5);
    CHECK(std::string(s.c_str()) == "Hello");
    CHECK_FALSE(s.is_truncated());
}

TEST_CASE("append: appending to an already-full string fails cleanly") {
    FixedString<5> s("abcd"); // already at usable capacity
    int rc = s.append("more");
    CHECK(rc == -1);
    CHECK(std::string(s.c_str()) == "abcd"); // unchanged
}

TEST_CASE("replace: content exactly fills the target range") {
    FixedString<32> s("Hello, World!");
    int rc = s.replace(7, 10, "XYZ"); // range is exactly 3 chars wide
    CHECK(rc == 0);                    // exact fit must report success, not truncation
    CHECK(std::string(s.c_str()) == "Hello, XYZld!");
    CHECK_FALSE(s.is_truncated());
}

TEST_CASE("replace: content shorter than the range overwrites only the prefix") {
    FixedString<32> s("Hello, World!");
    int rc = s.replace(7, 10, "X");
    CHECK(rc == 0);
    CHECK(std::string(s.c_str()) == "Hello, Xorld!"); // no shift/erase — fixed-width overwrite
    CHECK_FALSE(s.is_truncated());
}

TEST_CASE("replace: content longer than the range is truncated and flagged") {
    FixedString<32> s("Hello, World!");
    int rc = s.replace(7, 10, "?????");
    CHECK(rc == 1);                    // 1 == "wrote successfully, but content was dropped"
    CHECK(std::string(s.c_str()) == "Hello, ???ld!"); // only 3 of 5 '?' fit
    CHECK(s.is_truncated());
}

TEST_CASE("replace: zero-length range is a no-op that reports success") {
    FixedString<32> s("Hello, World!");
    int rc = s.replace(5, 5, "X");
    CHECK(rc == 0);
    CHECK(std::string(s.c_str()) == "Hello, World!"); // untouched
    CHECK_FALSE(s.is_truncated());
}

TEST_CASE("replace: invalid indices are rejected") {
    FixedString<32> s("Hello, World!");

    SUBCASE("negative start index") {
        CHECK(s.replace(-1, 3, "X") == -1);
    }
    SUBCASE("end index past the current string length") {
        CHECK(s.replace(0, 999, "X") == -1);
    }
    SUBCASE("start index greater than end index") {
        CHECK(s.replace(5, 2, "X") == -1);
    }
    SUBCASE("nullptr content") {
        CHECK(s.replace(0, 3, nullptr) == -1);
    }

    // None of the rejected calls should have mutated the string.
    CHECK(std::string(s.c_str()) == "Hello, World!");
    CHECK_FALSE(s.is_truncated());
}

TEST_CASE("truncation flag is sticky across multiple operations") {
    FixedString<10> s("1234"); // usable capacity 9

    CHECK_FALSE(s.is_truncated());
    s.append("567890"); // truncates: flag flips to true
    CHECK(s.is_truncated());

    // A later, perfectly successful operation must NOT clear the flag.
    s.replace(0, 1, "9");
    CHECK(s.is_truncated());
}