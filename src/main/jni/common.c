//
// Created by Thom on 2019/3/20.
//

#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <sys/system_properties.h>
#include "common.h"
#include "handle-error.h"
#include "plt.h"

static volatile int mGenuine;

static bool onCheckTrue(JNIEnv *env __unused) {
#ifdef DEBUG_NATIVE
    has_native_libs();
#endif
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
    return NULL;
#endif
}

__attribute__((__format__ (__printf__, 2, 0)))
void genuine_log_print(int prio, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    __android_log_vprint(prio, TAG, fmt, ap);
    va_end(ap);
}

static inline void fill_ro_build_version_sdk(char v[]) {
    // ro.build.version.sdk
    static unsigned int m = 0;

    if (m == 0) {
        m = 19;
    } else if (m == 23) {
        m = 29;
    }

    v[0x0] = 's';
    v[0x1] = 'm';
    v[0x2] = '-';
    v[0x3] = 'f';
    v[0x4] = 'p';
    v[0x5] = 'o';
    v[0x6] = 'k';
    v[0x7] = 'l';
    v[0x8] = '\'';
    v[0x9] = '|';
    v[0xa] = 'n';
    v[0xb] = '~';
    v[0xc] = '~';
    v[0xd] = 'g';
    v[0xe] = '`';
    v[0xf] = '~';
    v[0x10] = '?';
    v[0x11] = 'a';
    v[0x12] = 'd';
    v[0x13] = 'j';
    for (unsigned int i = 0; i < 0x14; ++i) {
        v[i] ^= ((i + 0x14) % m);
    }
    v[0x14] = '\0';
}

int getSdk() {
    static int sdk = 0;
    if (sdk == 0) {
        char v1[0x20];
        char prop[PROP_VALUE_MAX] = {0};
        fill_ro_build_version_sdk(v1);
        __system_property_get(v1, prop);
        sdk = (int) strtol(prop, NULL, 10);
    }
    return sdk;
}

#ifdef DEBUG

void debug(JNIEnv *env, const char *format, jobject object) {
    if (object == NULL) {
        LOGI(format, NULL);
    } else {
        jclass objectClass = (*env)->GetObjectClass(env, object);
        jmethodID toString = (*env)->GetMethodID(env, objectClass, "toString",
                                                 "()Ljava/lang/String;");
        jstring string = (jstring) (*env)->CallObjectMethod(env, object, toString);
        const char *value = (*env)->GetStringUTFChars(env, string, NULL);
        LOGI(format, value);
        (*env)->ReleaseStringUTFChars(env, string, value);
        (*env)->DeleteLocalRef(env, string);
        (*env)->DeleteLocalRef(env, objectClass);
    }
}

#endif

static void inline fill_NewLocalRef(char v[]) {
    // _ZN3art9JNIEnvExt11NewLocalRefEPNS_6mirror6ObjectE
    static unsigned int m = 0;

    if (m == 0) {
        m = 47;
    } else if (m == 53) {
        m = 59;
    }

    v[0x0] = '\\';
    v[0x1] = '^';
    v[0x2] = 'K';
    v[0x3] = '5';
    v[0x4] = 'f';
    v[0x5] = 'z';
    v[0x6] = '}';
    v[0x7] = '3';
    v[0x8] = 'A';
    v[0x9] = 'B';
    v[0xa] = 'D';
    v[0xb] = 'K';
    v[0xc] = 'a';
    v[0xd] = 'f';
    v[0xe] = 'T';
    v[0xf] = 'j';
    v[0x10] = 'g';
    v[0x11] = '%';
    v[0x12] = '$';
    v[0x13] = 'X';
    v[0x14] = 'r';
    v[0x15] = 'o';
    v[0x16] = 'U';
    v[0x17] = 'u';
    v[0x18] = 'x';
    v[0x19] = '}';
    v[0x1a] = 'q';
    v[0x1b] = 'L';
    v[0x1c] = 'z';
    v[0x1d] = 'F';
    v[0x1e] = 'd';
    v[0x1f] = 'r';
    v[0x20] = 'm';
    v[0x21] = 'w';
    v[0x22] = 'z';
    v[0x23] = '\x10';
    v[0x24] = 'J';
    v[0x25] = 'A';
    v[0x26] = '[';
    v[0x27] = 'X';
    v[0x28] = 'D';
    v[0x29] = '^';
    v[0x2a] = '\x1b';
    v[0x2b] = 'a';
    v[0x2c] = 'b';
    v[0x2d] = 'k';
    v[0x2e] = 'g';
    v[0x2f] = '`';
    v[0x30] = 'p';
    v[0x31] = '@';
    for (unsigned int i = 0; i < 0x32; ++i) {
        v[i] ^= ((i + 0x32) % m);
    }
    v[0x32] = '\0';
}

jobject newLocalRef(JNIEnv *env, void *object) {
    static jobject (*NewLocalRef)(JNIEnv *, void *) = NULL;
    if (object == NULL) {
        return NULL;
    }
    if (NewLocalRef == NULL) {
        char v[0x40];
        fill_NewLocalRef(v);
        NewLocalRef = (jobject (*)(JNIEnv *, void *)) plt_dlsym(v, NULL);
#ifdef DEBUG
        LOGI("NewLocalRef: %p", NewLocalRef);
#endif
    }
    if (NewLocalRef != NULL) {
        return NewLocalRef(env, object);
    } else {
        return NULL;
    }
}

static inline void fill_DeleteLocalRef(char v[]) {
    // _ZN3art9JNIEnvExt14DeleteLocalRefEP8_jobject
    static unsigned int m = 0;

    if (m == 0) {
        m = 43;
    } else if (m == 47) {
        m = 53;
    }

    v[0x0] = '^';
    v[0x1] = 'X';
    v[0x2] = 'M';
    v[0x3] = '7';
    v[0x4] = 'd';
    v[0x5] = 't';
    v[0x6] = 's';
    v[0x7] = '1';
    v[0x8] = 'C';
    v[0x9] = 'D';
    v[0xa] = 'B';
    v[0xb] = 'I';
    v[0xc] = 'c';
    v[0xd] = 'x';
    v[0xe] = 'J';
    v[0xf] = 'h';
    v[0x10] = 'e';
    v[0x11] = '#';
    v[0x12] = '\'';
    v[0x13] = 'P';
    v[0x14] = 'p';
    v[0x15] = 'z';
    v[0x16] = 'r';
    v[0x17] = 'l';
    v[0x18] = '|';
    v[0x19] = 'V';
    v[0x1a] = 't';
    v[0x1b] = '\x7f';
    v[0x1c] = '|';
    v[0x1d] = 'r';
    v[0x1e] = 'M';
    v[0x1f] = 'E';
    v[0x20] = 'G';
    v[0x21] = 'g';
    v[0x22] = 's';
    v[0x23] = '\x1c';
    v[0x24] = 'z';
    v[0x25] = 'L';
    v[0x26] = 'H';
    v[0x27] = 'J';
    v[0x28] = 'C';
    v[0x29] = 'O';
    v[0x2a] = 'c';
    v[0x2b] = 'u';
    for (unsigned int i = 0; i < 0x2c; ++i) {
        v[i] ^= ((i + 0x2c) % m);
    }
    v[0x2c] = '\0';
}

void DeleteLocalRef(JNIEnv *env, jobject object) {
    static void (*DeleteLocalRef)(JNIEnv *, jobject) = NULL;
    if (DeleteLocalRef == NULL) {
        char v[0x30];
        fill_DeleteLocalRef(v);
        DeleteLocalRef = (void (*)(JNIEnv *, jobject)) plt_dlsym(v, NULL);
#ifdef DEBUG
        LOGI("DeleteLocalRef: %p", DeleteLocalRef);
#endif
    }
    if (DeleteLocalRef != NULL) {
        DeleteLocalRef(env, object);
    }
}