//
// Created by Thom on 2019/2/16.
//

#ifndef BREVENT_PLT_H
#define BREVENT_PLT_H

#include <elf.h>
#include <link.h>
#include <android/log.h>

#ifndef TAG
#define TAG "Genuine"
#endif

#define LOGI(...) (__android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__))
#define LOGW(...) (__android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__))

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Symbol {
    const char *symbol;
    ElfW(Addr) *symbol_plt;
    ElfW(Addr) *symbol_sym;
} Symbol;

__attribute__ ((visibility ("internal")))
int dl_iterate_phdr_symbol(Symbol *symbol, const char *name);

#ifdef __cplusplus
}
#endif

#endif //BREVENT_PLT_H
