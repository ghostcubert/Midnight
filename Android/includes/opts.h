#pragma once
#include <string>
#include <cstdint>

constexpr const char* BACKEND_URL = "http://127.0.0.1:8080";
constexpr bool BYPASS_SSL = false; // only works with curl versions (aka if it doesnt use processrequest i cba bypassing ssl for this)
constexpr bool ENABLE_LOGS = false; // to debug stuff js keep it disabled for release

// if you need any offsets either find them using ida or choose a version from offsets.txt file
namespace FCurlHookOpts {
    constexpr bool USE_PR = true; // only enable this if ur doing 18.40+
    constexpr uintptr_t PR_ADDR = 0xB98C690; // set this to the correct address (processrequest)
    constexpr uintptr_t GETURL_ADDR = 0x70; // set this to the correct address (geturl its mostly 0x70 until like s25/26+)
    constexpr uintptr_t SETURL_ADDR = 0xB98A238; // set this to the correct address (seturl)
}

namespace EOSFCurlHookOpts {
    constexpr bool USE_EOS = false; // only enable this if ur doing s22+
    constexpr uintptr_t PR_ADDR = 0x0; // set this to the correct address (processrequest)
    constexpr uintptr_t GETURL_ADDR = 0x0; // set this to the correct address (geturl its 0x178 in almost any build)
    constexpr uintptr_t SETURL_ADDR = 0x0; // set this to the correct address (seturl)
}
