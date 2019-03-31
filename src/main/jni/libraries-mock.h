//
// Created by Thom on 2019/3/3.
//

#ifndef BREVENT_LIBRARIES_MOCK_H
#define BREVENT_LIBRARIES_MOCK_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

char *dump_jvm(JavaVM *jvm, void (*DumpForSigQuit)(void *, void *));

#ifdef __cplusplus
}
#endif

#endif //BREVENT_LIBRARIES_MOCK_H
