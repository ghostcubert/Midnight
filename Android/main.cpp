#include <pthread.h>
#include "includes/opts.h"
#include "includes/hooks.h"

// most cleanest main.cpp i probably ever did
__attribute__((constructor))
void Main() {
    pthread_t ptid;
    if (FCurlHookOpts::USE_PR) {
        pthread_create(&ptid, nullptr, Hooks::FCurlHook::SetupHook, nullptr);
        if (EOSFCurlHookOpts::USE_EOS) {
            pthread_create(&ptid, nullptr, Hooks::EOSFCurlHook::SetupHook, nullptr);
        }
    } else {
        pthread_create(&ptid, nullptr, Hooks::CurlHook::SetupHook, nullptr);
    }
}