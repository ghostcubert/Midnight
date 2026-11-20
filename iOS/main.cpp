#include "protocol.h"
#include <objc/runtime.h>
#include <objc/message.h>
#include <Foundation/Foundation.h>

void register_midnight_protocol() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    // create class
    Class nsurlProtocol = (Class)objc_getClass("NSURLProtocol");
    if (!nsurlProtocol) return;
    Class midnightProtocol = objc_allocateClassPair(nsurlProtocol, "MidnightProtocol", 0);
    if (!midnightProtocol) return;

    // add protocol functions
    Class midnightMetaClass = object_getClass(midnightProtocol);
    class_addMethod(midnightMetaClass, sel_registerName("canInitWithRequest:"), (IMP)can_init_with_request, "B@:@");
    class_addMethod(midnightMetaClass, sel_registerName("canInitWithTask:"), (IMP)can_init_with_task, "B@:@");
    class_addMethod(midnightMetaClass, sel_registerName("canonicalRequestForRequest:"), (IMP)canonical_request_for_request, "@@:@");
    class_addMethod(midnightProtocol, sel_registerName("startLoading"), (IMP)start_loading, "v@:");
    class_addMethod(midnightProtocol, sel_registerName("stopLoading"), (IMP)stop_loading, "v@:");  

    // register
    objc_registerClassPair(midnightProtocol);
    id (*msgSend_id_id)(id, SEL, id) = (id (*)(id, SEL, id))objc_msgSend;
    msgSend_id_id((id)objc_getClass("NSURLProtocol"), sel_registerName("registerClass:"), (id)midnightProtocol);
}

__attribute__((constructor))
static void init() {
    register_midnight_protocol();
}