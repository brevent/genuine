#include "genuine_extra.h"

// FIXME: your own methods

__attribute__ ((visibility ("internal")))
jint JNI_OnLoad_Extra(JNIEnv *env __unused, jclass clazz __unused, int sdk __unused, bool *onError __unused) {
    // FIXME: register your own methods
    // return env->RegisterNatives(clazz, methods, NELEM(methods));
    return JNI_OK;
}
