#ifndef BREVENT_ANTI_XPOSED_H
#define BREVENT_ANTI_XPOSED_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

__attribute__ ((visibility ("internal")))
jboolean antiXposed(JNIEnv *env, jclass clazz, const char *maps, int sdk, bool *xposed);

__attribute__ ((visibility ("internal")))
bool antiEdXposed(JNIEnv *env);

#ifdef __cplusplus
}
#endif

#endif //BREVENT_ANTI_XPOSED_H
