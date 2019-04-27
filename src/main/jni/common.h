//
// Created by Thom on 2019/3/20.
//

#ifndef BREVENT_COMMON_H
#define BREVENT_COMMON_H

#include <jni.h>
#include <stdbool.h>
#include <android/log.h>

#if __has_include("genuine.h")
#include "genuine.h"
#else
#error "please define genuine.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CHECK_TRUE,
    CHECK_FALSE,
    CHECK_FAKE,
    CHECK_OVERLAY,
    CHECK_ODEX,
    CHECK_DEX,
    CHECK_PROXY,
    CHECK_ERROR,
    CHECK_FATAL,
    CHECK_NOAPK,
};

#ifndef TAG
#define TAG "Genuine"
#endif

#define LOGI(...) (genuine_log_print(ANDROID_LOG_INFO, __VA_ARGS__))
#define LOGW(...) (genuine_log_print(ANDROID_LOG_WARN, __VA_ARGS__))
#define LOGE(...) (genuine_log_print(ANDROID_LOG_ERROR, __VA_ARGS__))

void genuine_log_print(int prio, const char *fmt, ...);

char *getGenuinePackageName();

char *getGenuineClassName();

bool setGenuine(JNIEnv *env, int genuine);

int getGenuine();

#ifdef __cplusplus
}
#endif

#endif //BREVENT_COMMON_H
