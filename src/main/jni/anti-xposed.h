#ifndef BREVENT_ANTI_XPOSED_H
#define BREVENT_ANTI_XPOSED_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

jboolean antiXposed(JNIEnv *env, jclass clazz, int sdk, bool *xposed);

bool doAntiEdXposed(JNIEnv *env, jobject classLoader);

#ifdef __cplusplus
}
#endif

#endif //BREVENT_ANTI_XPOSED_H
