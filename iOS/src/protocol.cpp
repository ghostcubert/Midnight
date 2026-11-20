#include "protocol.h"
#include <cstring>
#include <string>
#include <vector>

namespace {

using RequestFn = id (*)(id, SEL, id, id);
using SetNavFn = void (*)(id, SEL, id);
using NavFn = void (*)(id, SEL, id, id, id);

RequestFn request_method = nullptr;
SetNavFn nav_method = nullptr;

struct NavMethod {
    Class cls;
    NavFn method;
};

std::vector<NavMethod> nav_methods;

const char *get_utf8(id string) {
    if (!string) return nullptr;
    return ((const char *(*)(id, SEL))objc_msgSend)(string, sel_registerName("UTF8String"));
}

std::string nsstring_to_std(id string) {
    const char *value = get_utf8(string);
    return value ? std::string(value) : std::string();
}

id std_to_nsstring(const char *value) {
    return ((id (*)(id, SEL, const char *))objc_msgSend)(
        (id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), value);
}

id std_to_nsstring(const std::string &value) {
    return std_to_nsstring(value.c_str());
}

bool is_http_url(id url) {
    id scheme = ((id (*)(id, SEL))objc_msgSend)(url, sel_registerName("scheme"));
    std::string value = nsstring_to_std(scheme);
    return value == "http" || value == "https";
}

// redirect graphql
void redirect_request(id self, SEL cmd, id request, id completion_handler) {
    if (!request_method) return;
    id outgoing_request = request;

    if (request) {
        id url = ((id (*)(id, SEL))objc_msgSend)(request, sel_registerName("URL"));
        id absolute_string = url ? ((id (*)(id, SEL))objc_msgSend)(url, sel_registerName("absoluteString")) : nullptr;
        std::string absolute = nsstring_to_std(absolute_string);

        if (absolute.find("graphql.epicgames.com") != std::string::npos) {
            id path_object = ((id (*)(id, SEL))objc_msgSend)(url, sel_registerName("path"));
            id query_object = ((id (*)(id, SEL))objc_msgSend)(url, sel_registerName("query"));
            std::string redirected_url = BACKEND_URL;
            redirected_url += nsstring_to_std(path_object);
            if (query_object) {
                redirected_url += "?";
                redirected_url += nsstring_to_std(query_object);
            }

            id ns_url = ((id (*)(id, SEL, id))objc_msgSend)(
                (id)objc_getClass("NSURL"), sel_registerName("URLWithString:"), std_to_nsstring(redirected_url));
            id mutable_request = ((id (*)(id, SEL))objc_msgSend)(request, sel_registerName("mutableCopy"));
            if (ns_url && mutable_request) {
                ((void (*)(id, SEL, id))objc_msgSend)(mutable_request, sel_registerName("setURL:"), ns_url);
                outgoing_request = mutable_request;
            }
        }
    }

    request_method(self, cmd, outgoing_request, completion_handler);
}

NavMethod *find_hook(Class cls) {
    for (NavMethod &hook : nav_methods) {
        if (hook.cls == cls) return &hook;
    }
    return nullptr;
}

void nav_policy(id self, SEL cmd, id web_view, id navigation_action, id decision_handler) {
    Class cls = (Class)((id (*)(id, SEL))objc_msgSend)(self, sel_registerName("class"));
    NavMethod *hook = find_hook(cls);
    if (!hook || !hook->method) return;
    NavFn original = hook->method;

    id request = navigation_action
        ? ((id (*)(id, SEL))objc_msgSend)(navigation_action, sel_registerName("request")) : nullptr;
    id url = request ? ((id (*)(id, SEL))objc_msgSend)(request, sel_registerName("URL")) : nullptr;

    if (url && is_http_url(url)) {
        id absolute_string = ((id (*)(id, SEL))objc_msgSend)(url, sel_registerName("absoluteString"));
        std::string absolute = nsstring_to_std(absolute_string);
        const char *target = nullptr;
        if (absolute.find("/id/login") != std::string::npos) {
            target = PARTYHUB_LOGIN_URL;
        } else if (absolute.find("/id/register") != std::string::npos) {
            target = PARTYHUB_REGISTER_URL;
        }

        if (target) {
            using DecisionBlock = void (^)(long);
            ((DecisionBlock)decision_handler)(1); // WKNavigationActionPolicyCancel
            id target_url = ((id (*)(id, SEL, id))objc_msgSend)(
                (id)objc_getClass("NSURL"), sel_registerName("URLWithString:"), std_to_nsstring(target));
            id target_request = ((id (*)(id, SEL, id))objc_msgSend)(
                (id)objc_getClass("NSURLRequest"), sel_registerName("requestWithURL:"), target_url);
            if (target_request) {
                ((void (*)(id, SEL, id))objc_msgSend)(web_view, sel_registerName("loadRequest:"), target_request);
            }
            return;
        }
    }
    original(self, cmd, web_view, navigation_action, decision_handler);
}

void set_nav(id self, SEL cmd, id delegate_obj) {
    if (delegate_obj) {
        Class cls = (Class)((id (*)(id, SEL))objc_msgSend)(delegate_obj, sel_registerName("class"));
        if (!find_hook(cls)) {
            SEL selector = sel_registerName("webView:decidePolicyForNavigationAction:decisionHandler:");
            Method method = class_getInstanceMethod(cls, selector);
            if (method) {
                IMP old_method = method_getImplementation(method);
                nav_methods.push_back({cls, (NavFn)old_method});
                method_setImplementation(method, (IMP)nav_policy);
            }
        }
    }

    if (nav_method) {
        nav_method(self, cmd, delegate_obj);
    }
}

}

void start_partyhub(void) {
    Class session_class = (Class)objc_getClass("NSURLSession");
    SEL data_task_selector = sel_registerName("dataTaskWithRequest:completionHandler:");
    Method data_task_method = session_class ? class_getInstanceMethod(session_class, data_task_selector) : nullptr;
    if (data_task_method) {
        request_method = (RequestFn)method_getImplementation(data_task_method);
        method_setImplementation(data_task_method, (IMP)redirect_request);
    }

    Class web_view_class = (Class)objc_getClass("WKWebView");
    SEL set_delegate_selector = sel_registerName("setNavigationDelegate:");
    Method set_delegate_method = web_view_class ? class_getInstanceMethod(web_view_class, set_delegate_selector) : nullptr;
    if (set_delegate_method) {
        nav_method = (SetNavFn)method_getImplementation(set_delegate_method);
        method_setImplementation(set_delegate_method, (IMP)set_nav);
    }
}

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

    const char* EPIC_DOMAINS[] = {
        "game-social.epicgames.com",
        "ol.epicgames.com",
        "ol.epicgames.net",
        "on.epicgames.com",
        "ak.epicgames.com",
        "epicgames.dev"
    };

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
