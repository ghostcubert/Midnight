#pragma once

#include "opts.h"
#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>
#include <objc/runtime.h>
#include <objc/message.h>

#ifdef __cplusplus
extern "C"
{
#endif
    BOOL can_init_with_request(Class self, SEL cmd, id request);
    BOOL can_init_with_task(Class self, SEL cmd, id task);
    id canonical_request_for_request(Class self, SEL cmd, id request);
    void start_loading(id self, SEL cmd);
    void stop_loading(id self, SEL cmd);
#ifdef __cplusplus
}
#endif
