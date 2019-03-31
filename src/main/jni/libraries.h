//
// Created by Thom on 2019/2/16.
//

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

jobject findClassLoader(JNIEnv *env, const char *name, int sdk);

#ifdef __cplusplus
}
#endif
