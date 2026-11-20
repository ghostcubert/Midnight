#pragma once
#include <cstddef>
#include "includes/FString.h"
#include "includes/opts.h"
#include "includes/url.h"

namespace Hooks {
    namespace InternalHooks {
        bool InternalProcessRequest(void* Req, bool isEOS);
    }

    namespace CurlHook {
        extern int (*OG_SETOPT)(void*, int, void*);
        void* SetupHook(void*);
        int SetOptHook(void* handle, int option, void* args);
    }

    struct FCurlHook {
        static void (*SetUrl)(void*, const void*);
        static bool (*OGProcessRequest)(void*);

        FString::FString& GetUrl() { return *(FString::FString*)((uintptr_t)this + FCurlHookOpts::GETURL_ADDR); }
        static bool ProcessRequest(FCurlHook* Req) { return InternalHooks::InternalProcessRequest(Req, false); };
        static void* SetupHook(void* handle);
    };

    struct EOSFCurlHook {
        static void (*SetUrl)(void*, const void*);
        static bool (*OGProcessRequest)(void*);

        FString::FString& GetUrl() { return *(FString::FString*)((uintptr_t)this + EOSFCurlHookOpts::GETURL_ADDR); }
        static bool ProcessRequest(EOSFCurlHook* Req) { return InternalHooks::InternalProcessRequest(Req, true); };
        static void* SetupHook(void* handle);
    };
}
