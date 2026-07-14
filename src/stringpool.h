#pragma once

// Shared declarations for StringPool operations.
// The implementation lives in stringpool.cpp.
// ============================================================================

constexpr size_t GetLength(const char* s) {
    size_t n = 0;
    while (s[n]) {
        ++n;
    }
    return n;
}

void EncodeString(int nIdx, const char* sSource, char* sDestination);

#define REPLACE_STRING(INDEX, NEW_STRING) \
    do { \
        static char sEncoded[GetLength(NEW_STRING) + 2]; \
        EncodeString(INDEX, NEW_STRING, sEncoded); \
    } while (0)
