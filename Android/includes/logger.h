#pragma once
#include <android/log.h>
#include "includes/opts.h"

#define LOG_TAG "midnight"

#define LOGI(...) do { if constexpr (ENABLE_LOGS) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__); } while(0)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
