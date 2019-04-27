//
// Created by Thom on 2019/3/20.
//

#include <jni.h>
#include <string.h>
#include "common.h"
#include "handle-error.h"

static volatile int mGenuine;

static bool onCheckTrue(JNIEnv *env __unused) {
#ifdef DEBUG_GENUINE_MOCK
    start_native_activity_async(env);
#endif
    return true;
}

static bool onCheckFalse(JNIEnv *env __unused) {
#if defined(GENUINE_FALSE_CRASH)
    return false;
#elif defined(GENUINE_FALSE_NATIVE)
    start_native_activity_async(env);
#endif
    return true;
}

static bool onCheckFake(JNIEnv *env __unused) {
#if defined(GENUINE_FAKE_CRASH)
    return false;
#elif defined(GENUINE_FAKE_NATIVE)
    start_native_activity_async(env);
#endif
    return true;
}

static bool onCheckOverlay(JNIEnv *env __unused) {
#if defined(GENUINE_OVERLAY_CRASH)
    return false;
#elif defined(GENUINE_OVERLAY_NATIVE)
    start_native_activity_async(env);
#endif
    return true;
}

static bool onCheckOdex(JNIEnv *env __unused) {
#if defined(GENUINE_ODEX_CRASH)
    return false;
#elif defined(GENUINE_ODEX_NATIVE)
    start_native_activity_async(env);
#endif
    return true;
}

static bool onCheckDex(JNIEnv *env __unused) {
#if defined(GENUINE_DEX_CRASH)
    return false;
#elif defined(GENUINE_DEX_NATIVE)
    start_native_activity_async(env);
#endif
    return true;
}

static bool onCheckProxy(JNIEnv *env __unused) {
#if defined(GENUINE_PROXY_CRASH)
    return false;
#elif defined(GENUINE_PROXY_NATIVE)
    start_native_activity_async(env);
#endif
    return true;
}

static bool onCheckError(JNIEnv *env __unused) {
#if defined(GENUINE_ERROR_CRASH)
    return false;
#elif defined(GENUINE_ERROR_NATIVE)
    start_native_activity_async(env);
#endif
    return true;
}

static bool onCheckFatal(JNIEnv *env __unused) {
#if defined(GENUINE_FALTAL_CRASH)
    return false;
#elif defined(GENUINE_FATAL_NATIVE)
    start_native_activity_async(env);
#endif
    return true;
}

static bool onCheckNoapk(JNIEnv *env __unused) {
#if defined(GENUINE_NOAPK_CRASH)
    return false;
#elif defined(GENUINE_NOAPK_NATIVE)
    start_native_activity_async(env);
#endif
    return true;
}

bool setGenuine(JNIEnv *env, int genuine) {
    mGenuine = genuine;
    switch (genuine) {
        case CHECK_TRUE:
            return onCheckTrue(env);
        case CHECK_FALSE:
            return onCheckFalse(env);
        case CHECK_FAKE:
            return onCheckFake(env);
        case CHECK_OVERLAY:
            return onCheckOverlay(env);
        case CHECK_ODEX:
            return onCheckOdex(env);
        case CHECK_DEX:
            return onCheckDex(env);
        case CHECK_PROXY:
            return onCheckProxy(env);
        case CHECK_ERROR:
            return onCheckError(env);
        case CHECK_FATAL:
            return onCheckFatal(env);
        case CHECK_NOAPK:
            return onCheckNoapk(env);
        default:
            return true;
    }
}

int getGenuine() {
    return mGenuine;
}

char *getGenuineClassName() {
#ifdef GET_GENUINE_CLASS_NAME
    return GET_GENUINE_CLASS_NAME();
#else
#ifndef GENUINE_CLAZZ
#define GENUINE_CLAZZ "me/piebridge/Genuine"
#endif
    return strdup(GENUINE_CLAZZ);
#endif
}

char *getGenuinePackageName() {
#ifdef GET_GENUINE_PACKAGE_NAME
    return GET_GENUINE_PACKAGE_NAME();
#elif defined(GENUINE_NAME)
    static unsigned int m = 0;
    if (m == 0) {
        m = 20;
    } else if (m == 23) {
        m = 29;
    }
    char name[] = GENUINE_NAME;
    unsigned int length = sizeof(name) - 1;
    for (unsigned int i = 0; i < length; ++i) {
        name[i] ^= ((i + length) % m);
    }
    name[length] = '\0';
    return strdup(name);
#else
#error "specify GET_GENUINE_PACKAGE_NAME or GENUINE_NAME"
#endif
}

__attribute__((__format__ (__printf__, 2, 0)))
void genuine_log_print(int prio, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    __android_log_vprint(prio, TAG, fmt, ap);
    va_end(ap);
}
