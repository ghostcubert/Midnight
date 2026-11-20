LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := midnight
LOCAL_SRC_FILES := main.cpp src/hooks.cpp src/FString.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/includes/dobby
LOCAL_STATIC_LIBRARIES := dobby
LOCAL_CPPFLAGS := -std=c++20 -fvisibility=hidden -fPIC
LOCAL_LDLIBS := -llog -ldl
LOCAL_MODULE_FILENAME := libmidnight

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := dobby
LOCAL_SRC_FILES := includes/dobby/libdobby.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/includes/dobby
include $(PREBUILT_STATIC_LIBRARY)
