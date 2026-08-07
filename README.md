# FixedString

A fixed-capacity, allocation-free string type for C++, designed for
embedded and other resource-constrained environments where heap
allocation is unavailable, undesirable, or non-deterministic.

`FixedString<N>` stores up to `N - 1` characters in an inline `char[N]`
buffer plus a length counter.

Capacity is fixed at compile time via the template
parameter and enforced on every write.

## Usage

```cpp
#include "FixedString.h"

FixedString<32> s("Hello, World!");

s.size();          // 13
s.capacity();       // 31 (usable chars - see below)
s.c_str();          // "Hello, World!"
s.is_truncated();   // false

s.append("!!!");    // grows the string in place
s.replace(7, 10, "XYZ"); // overwrites indices [7, 10)
```

## API

| Method | Behavior |
|---|---|
| `FixedString(const char* content)` | Constructs from a C string. `nullptr` yields an empty string. Content longer than capacity is truncated. |
| `size()` | Current string length, in characters (excludes the null terminator). |
| `capacity()` | Maximum usable string length - this is `N - 1`, not `N` (see [Design notes](#design-notes)). |
| `is_truncated()` | `true` if *any* operation on this object has ever dropped content. Sticky so never resets. |
| `append(const char* content)` | Appends to the end of the string, up to available capacity. Returns `0` on full success, `-1` if content had to be truncated. |
| `replace(int start, int end, const char* content)` | Overwrites `[start, end)` in place with `content`. Does **not** shift or resize the string (see [Design notes](#design-notes)). Returns `0` on exact fit, `1` if `content` was longer than the range and got truncated, `-1` on invalid arguments. |
| `c_str()` | Returns the underlying null-terminated buffer. |

## Design notes

A few behaviors here are deliberate choices I made.

**`capacity()` excludes the null terminator.**
`FixedString<N>` allocates `N` bytes total, but one byte is always
reserved for the null terminator so `c_str()` is guaranteed valid.
`capacity()` reports `N - 1`, the number of actual characters you can
store and not the raw buffer size.

**`replace()` is a fixed-width overwrite, not `std::string::replace`.**
`std::string::replace` lets the replacement content be any length -
the string grows or shrinks and the tail shifts to match. `FixedString`
can't do that safely within a fixed buffer without a lot of extra
complexity, so `replace(start, end, content)` instead overwrites the
existing range in place:

- If `content` is shorter than the range, only the first part of the
  range is overwritten.
- If `content` is longer than the range, the extra characters are
  silently dropped and `is_truncated()` becomes `true`.
- The string's `size()` never changes as a result of `replace()`.

**The truncation flag is sticky.**
`is_truncated()` is `true` if *any* construction, `append`, or
`replace` call has ever lost data. It does not reset on a
subsequent successful operation. Check the return value of individual calls if you need
per-operation granularity instead.

## Building and testing

Tests are written with [doctest](https://github.com/doctest/doctest)
(single header, included in this repo).

```sh
g++ test.cpp -o testOut
.testOut
```

15 test cases / 58 assertions covering construction, `append`,
`replace`, invalid-argument handling, and truncation-flag behavior
across multiple operations, including several boundary cases (exact
capacity fit, zero-length ranges, appending to an already-full
string).

## Limitations / possible future work

- No `operator[]`, iterators, or comparison operators yet.
- No `std::string_view` interop.
