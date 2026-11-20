#pragma once
#include <cstdint>

namespace FString {
    using TCHAR = char16_t;

    struct FString
    {
        TCHAR*  data;
        int32_t length;
        int32_t capacity;
    };

    char* ToCStr(const FString* str);
    FString FromCStr(const char* str);
    void Free(FString* str);
}