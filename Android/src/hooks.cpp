#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include "includes/hooks.h"
#include "includes/dobby/dobby.h"
#include "includes/logger.h"
#include "includes/url.h"
#include "includes/opts.h"

static void* GetLibBase(const char* libName) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return nullptr;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, libName)) {
            uintptr_t base = strtoull(line, nullptr, 16);
            fclose(fp);
            return (void*)base;
        }
    }

    fclose(fp);
    return nullptr;
}

// ud hooks
namespace Hooks {
    namespace CurlHook {
        int (*OG_SETOPT)(void*, int, void*) = nullptr;
        void* SetupHook(void*) {
            LOGI("Loading Unreal lib...");
            void* handle = dlopen("libUE4.so", 2);
            if (!handle) {
                LOGE("Failed to load Unreal lib!");
                return nullptr;
            }

            void* SetOptSym = dlsym(handle, "curl_easy_setopt");
            if (!SetOptSym) {
                LOGE("Failed to find curl_easy_setopt symbol.");
                dlclose(handle);
                return nullptr;
            }

            if (DobbyHook(SetOptSym, (void*)SetOptHook, (void**)&OG_SETOPT) == 0) {
                LOGI("Successfully hooked curl_easy_setopt!");
            } else {
                LOGE("Failed to hook curl_easy_setopt!");
            }

            return nullptr;
        }

        int SetOptHook(void* handle, int option, void* args) {
            if (!OG_SETOPT)
                return -1;

            if (option == 10002 && args != nullptr) { // url hook
                const char* UrlCStr = static_cast<const char*>(args);
                if (UrlCStr) {
                    Url url = Url::ParseUrl(UrlCStr);
                    if (Url::ShouldRedirect(url.host)) {
                        thread_local std::string newUrl;
                        newUrl = Url::CreateUrl(BACKEND_URL, url.pathAndQuery);
                        LOGI("Redirected Request from %s to %s", UrlCStr, newUrl.c_str());
                        return OG_SETOPT(handle, option, (void*)newUrl.c_str());
                    }
                }
            } else if (option == 64 && BYPASS_SSL) { // SSL Bypass
                return OG_SETOPT(handle, option, reinterpret_cast<void*>(static_cast<intptr_t>(0)));
            }

            return OG_SETOPT(handle, option, args);
        }
    }

    bool (*FCurlHook::OGProcessRequest)(void*) = nullptr;
    void (*FCurlHook::SetUrl)(void*, const void*) = nullptr;

    void* FCurlHook::SetupHook(void* handle) {
        auto libName = "libUnreal.so"; // idk change this to libUE4.so if ur doing s18
        handle = dlopen(libName, 2);
        if (!handle) return nullptr;

        void* base = GetLibBase(libName);
        if (!base) return nullptr;

        SetUrl = (void (*)(void*, const void*))((char*)base + FCurlHookOpts::SETURL_ADDR);
        void* processRequestPtr = (char*)base + FCurlHookOpts::PR_ADDR;
        if (DobbyHook(processRequestPtr, (void*)FCurlHook::ProcessRequest, (void**)&OGProcessRequest) == 0) {
            LOGI("Successfully hooked Unreal's ProcessRequest!");
        } else {
            LOGE("Failed to hook Unreal's ProcessRequest.");
        }

        return nullptr;
    }

    bool (*EOSFCurlHook::OGProcessRequest)(void*) = nullptr;
    void (*EOSFCurlHook::SetUrl)(void*, const void*) = nullptr;

    void* EOSFCurlHook::SetupHook(void* handle) {
        auto libName = "libEOSSDK.so";
        handle = dlopen(libName, 2);
        if (!handle) return nullptr;

        void* base = GetLibBase(libName);
        if (!base) return nullptr;

        SetUrl = (void (*)(void*, const void*))((char*)base + EOSFCurlHookOpts::SETURL_ADDR);
        void* processRequestPtr = (char*)base + EOSFCurlHookOpts::PR_ADDR;
        if (DobbyHook(processRequestPtr, (void*)EOSFCurlHook::ProcessRequest, (void**)&OGProcessRequest) == 0) {
            LOGI("Successfully hooked EOS's ProcessRequest!");
        } else {
            LOGE("Failed to hook EOS's ProcessRequest.");
        }

        return nullptr;
    }

    namespace InternalHooks {
        bool InternalProcessRequest(void* Req, bool isEOS) {
            if (!Req) return false;

            // getting functions
            auto OGProcessRequest = isEOS ? EOSFCurlHook::OGProcessRequest : FCurlHook::OGProcessRequest;
            auto SetUrl = isEOS ? EOSFCurlHook::SetUrl : FCurlHook::SetUrl;

            // scuffed url getter
            FString::FString& OGUrl = isEOS ? ((EOSFCurlHook*)Req)->GetUrl() : ((FCurlHook*)Req)->GetUrl();
            if (OGUrl.length == 0 || !OGUrl.data)
                return OGProcessRequest(Req);

            char* OGUrlCStr = FString::ToCStr(&OGUrl);
            if (!OGUrlCStr) return OGProcessRequest(Req);

            // proper url setter
            Url url = Url::ParseUrl(OGUrlCStr);
            if (Url::ShouldRedirect(url.host)) {
                std::string newUrlCStr = Url::CreateUrl(BACKEND_URL, url.pathAndQuery);
                LOGI("Redirected Request from %s to %s", OGUrlCStr, newUrlCStr.c_str());
                auto* newUrl = (FString::FString*)malloc(sizeof(FString::FString));
                *newUrl = FString::FromCStr(newUrlCStr.c_str());
                SetUrl(Req, newUrl);
            }

            free(OGUrlCStr);
            return OGProcessRequest(Req);
        }
    }
}