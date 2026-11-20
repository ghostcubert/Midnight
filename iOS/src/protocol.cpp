#include "protocol.h"
#include <cstring>

// very ud protocol
BOOL can_init_with_request(Class self, SEL cmd, id req) {
    if (!req)
        return NO;

    id (*msgSend_id)(id, SEL) = (id (*)(id, SEL))objc_msgSend;
    id url = msgSend_id(req, sel_registerName("URL"));
    if (!url)
        return NO;

    id abs_url = msgSend_id(url, sel_registerName("absoluteString"));
    if (!abs_url)
        return NO;

    const char* (*msgSend_cstr)(id, SEL) = (const char* (*)(id, SEL))objc_msgSend;
    const char* cstr = msgSend_cstr(abs_url, sel_registerName("UTF8String"));
    if (!cstr)
        return NO;

    for (const char* domain : EPIC_DOMAINS) {
        if (strstr(cstr, domain) != nullptr) {
            return YES;
        }
    }

    return NO;
}

BOOL can_init_with_task(Class self, SEL cmd, id task) {
    if (!task)
        return NO;

    id (*msgSend_id)(id, SEL) = (id (*)(id, SEL))objc_msgSend;
    id req = msgSend_id(task, sel_registerName("currentRequest"));
    if (!req) {
        req = msgSend_id(task, sel_registerName("originalRequest"));
        if (!req)
            return NO;
    }

    return can_init_with_request(self, sel_registerName("canInitWithRequest:"), req);
}

id canonical_request_for_request(Class self, SEL cmd, id request) {
    return request;
}

void start_loading(id self, SEL cmd) {
    if (!self)
        return;

    id (*msgSend_id)(id, SEL) = (id (*)(id, SEL))objc_msgSend;
    id (*msgSend_id_id)(id, SEL, id) = (id (*)(id, SEL, id))objc_msgSend;
    void (*msgSend_void_id)(id, SEL, id) = (void (*)(id, SEL, id))objc_msgSend;
    void (*msgSend_void_id_id)(id, SEL, id, id) = (void (*)(id, SEL, id, id))objc_msgSend;

    id og_req = msgSend_id(self, sel_registerName("request"));
    if (!og_req) return;
    id request = msgSend_id(og_req, sel_registerName("mutableCopy"));
    if (!request) return;
    id url = msgSend_id(request, sel_registerName("URL"));
    if (!url) return;
    id ns_backend = ((id (*)(id, SEL, const char*))objc_msgSend)((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), BACKEND_URL);
    id components = msgSend_id_id((id)objc_getClass("NSURLComponents"), sel_registerName("componentsWithString:"), ns_backend);
    if (!components) return;
    id path = msgSend_id(url, sel_registerName("path"));
    id query = msgSend_id(url, sel_registerName("query"));

    msgSend_void_id(components, sel_registerName("setPath:"), path);
    msgSend_void_id(components, sel_registerName("setQuery:"), query);

    id new_url = msgSend_id(components, sel_registerName("URL"));
    if (!new_url) return;
    msgSend_void_id(request, sel_registerName("setURL:"), new_url);

    id client = msgSend_id(self, sel_registerName("client"));
    if (!client) return;

    ((void (*)(id, SEL, id, id, id))objc_msgSend)(
        client,
        sel_registerName("URLProtocol:wasRedirectedToRequest:redirectResponse:"),
        self,
        request,
        nullptr
    );

    ((void (*)(id, SEL))objc_msgSend)(request, sel_registerName("release"));
}

void stop_loading(id self, SEL cmd) {
    // idk what to put here...
}