#ifndef GENUINE_EXTRA_H
#define GENUINE_EXTRA_H

#include <jni.h>
#include <stdbool.h>

#ifndef NELEM
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// FIXME: your own methods

__attribute__ ((visibility ("internal")))
jint JNI_OnLoad_Extra(JNIEnv *env, jclass clazz, int sdk, bool *onError);

#ifdef __cplusplus
}
#endif

#endif //GENUINE_EXTRA_H
