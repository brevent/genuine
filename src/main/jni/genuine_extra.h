#ifndef GENUINE_EXTRA_H
#define GENUINE_EXTRA_H

#include <jni.h>

#ifndef NELEM
#define NELEM(x) static_cast<int>(sizeof(x) / sizeof((x)[0]))
#endif

__attribute__ ((visibility ("internal")))
jint JNI_OnLoad_Extra(JNIEnv *, jclass);

#endif //GENUINE_EXTRA_H
