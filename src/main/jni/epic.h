#ifndef EPIC_H
#define EPIC_H

#include <jni.h>
#include <stdbool.h>
#include "genuine.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SUPPORT_EPIC

__attribute__ ((visibility ("internal")))
bool antiEpic(JNIEnv *env, int sdk);

#endif

__attribute__ ((visibility ("internal")))
void clearHandler(JNIEnv *env);

#ifdef __cplusplus
}
#endif

#endif