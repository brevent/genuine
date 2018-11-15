#include "genuine_extra.h"

// FIXME: your own methods

__attribute__ ((visibility ("internal")))
jint JNI_OnLoad_Extra(JNIEnv *, jclass) {
    // FIXME: register your own methods
    // return env->RegisterNatives(clazz, methods, NELEM(methods));
    return JNI_OK;
}
