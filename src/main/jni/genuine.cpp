#include <stdio.h>
#include <string.h>
#include <jni.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/system_properties.h>
#include <inttypes.h>
#include <android/log.h>
#include <errno.h>

#if __has_include("genuine.h")
#include "genuine.h"
#else
#error "please define genuine.h"
#endif

#include "genuine_extra.h"

#ifndef TAG
#define TAG "Genuine"
#endif
#define LOGD(...) (__android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__))
#define LOGI(...) (__android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__))
#define LOGW(...) (__android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__))
#define LOGE(...) (__android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__))

enum {
    CHECK_ERROR,
    CHECK_UNKNOWN,
    CHECK_FALSE,
    CHECK_TRUE,
};

static int genuine;

static bool xposed = false;

static jmethodID original;

static jobject invoke(JNIEnv *env, jclass, jobject m, jint i, jobject, jobject t, jobjectArray as) {
    jobject result = env->CallStaticObjectMethod(NULL, original, m, i, NULL, NULL, t, as);
    if (env->ExceptionCheck()) {
        jthrowable throwable = env->ExceptionOccurred();
        env->ExceptionClear();
        jclass classInvokeException = env->FindClass("java/lang/reflect/InvocationTargetException");
        if (env->IsInstanceOf(throwable, classInvokeException)) {
            jclass classThrowable = env->FindClass("java/lang/Throwable");
            jthrowable cause = static_cast<jthrowable>(env->CallObjectMethod(throwable,
                env->GetMethodID(classThrowable, "getCause", "()Ljava/lang/Throwable;")));
            env->Throw(cause);
            env->DeleteLocalRef(cause);
            env->DeleteLocalRef(classThrowable);
        } else {
            env->Throw(throwable);
        }
        env->DeleteLocalRef(classInvokeException);
        env->DeleteLocalRef(throwable);
    }
    return result;
}

static jint version(JNIEnv *, jclass) {
    if (genuine == CHECK_TRUE) {
        return VERSION;
    } else {
        return 0;
    }
}

static inline void fill_proc_self_maps(char maps[]) {
    // /proc/self/maps
    maps[0x0] = ' ';
    maps[0x1] = '`';
    maps[0x2] = 'c';
    maps[0x3] = '}';
    maps[0x4] = 'p';
    maps[0x5] = ';';
    maps[0x6] = 'f';
    maps[0x7] = 's';
    maps[0x8] = '{';
    maps[0x9] = '~';
    maps[0xa] = '6';
    maps[0xb] = 'w';
    maps[0xc] = 'z';
    maps[0xd] = 'l';
    maps[0xe] = 'n';
    for (int i = 0; i < 0xf; ++i) {
        maps[i] ^= (i + 0xf);
    }
    maps[0xf] = '\0';
}

static inline bool islibartso(const char *str) {
    const char *dot = strrchr(str, 'l');
    return dot != NULL
           && *++dot == 'i'
           && *++dot == 'b'
           && *++dot == 'a'
           && *++dot == 'r'
           && *++dot == 't'
           && *++dot == '.'
           && *++dot == 's'
           && *++dot == 'o'
           && (*++dot == '\0' || *dot == '\r' || *dot == '\n');
}

static inline bool islibxposedart(const char *str) {
    const char *dot = strrchr(str, 'l');
    return dot != NULL
           && *++dot == 'i'
           && *++dot == 'b'
           && *++dot == 'x'
           && *++dot == 'p'
           && *++dot == 'o'
           && *++dot == 's'
           && *++dot == 'e'
           && *++dot == 'd'
           && *++dot == '_'
           && *++dot == 'a'
           && *++dot == 'r'
           && *++dot == 't'
           && *++dot == '.'
           && *++dot == 's'
           && *++dot == 'o'
           && (*++dot == '\0' || *dot == '\r' || *dot == '\n');
}

static inline jboolean checkXposed() {
    FILE *fp;
    char maps[16] = {0};
    jboolean check = JNI_FALSE;
    jboolean found = JNI_FALSE;

    fill_proc_self_maps(maps);

    fp = fopen(maps, "r");
    if (fp != NULL) {
        char line[PATH_MAX];
        while (fgets(line, PATH_MAX - 1, fp) != NULL) {
            if (islibxposedart(line)) {
                check = JNI_TRUE;
            } else if (islibartso(line)) {
                found = JNI_TRUE;
            }
            if (found && check) {
                break;
            }
        }
        fclose(fp);
    }

    if (found && check) {
        return JNI_TRUE;
    } else {
        return JNI_FALSE;
    }
}

static jmethodID *checkCallback(jmethodID value) {
    FILE *fp;
    char maps[16] = {0};

    fill_proc_self_maps(maps);

    fp = fopen(maps, "r");
    if (fp != NULL) {
        char line[PATH_MAX];
        uintptr_t start;
        uintptr_t end;
        char perm[5];
        int pos;
        while (fgets(line, PATH_MAX - 1, fp) != NULL) {
            if (sscanf(line, "%" PRIxPTR "-%" PRIxPTR " %4s %*" PRIxPTR " %*x:%*x %*d %n",
                       &start, &end, perm, &pos) == 0x3
                && perm[0] == 'r' && perm[1] == 'w'
                && islibartso(line + pos)) {
                for (jmethodID *s = reinterpret_cast<jmethodID *>(start); s < reinterpret_cast<jmethodID *>(end); ++s) {
                    if (*s == value) {
                        fclose(fp);
                        return s;
                    }
                }
            }
        }
        fclose(fp);
    }
    return NULL;
}

static int sdk() {
    char sdk[PROP_VALUE_MAX] = {0};
    __system_property_get("ro.build.version.sdk", sdk);
    return atoi(sdk);
}

static inline void fill_classLoader$SystemClassLoader(char v[]) {
    // java/lang/ClassLoader$SystemClassLoader
    v[0x0] = 'M';
    v[0x1] = 'I';
    v[0x2] = '_';
    v[0x3] = 'K';
    v[0x4] = '\x04';
    v[0x5] = '@';
    v[0x6] = 'L';
    v[0x7] = '@';
    v[0x8] = 'H';
    v[0x9] = '\x1f';
    v[0xa] = 'r';
    v[0xb] = '^';
    v[0xc] = 'R';
    v[0xd] = 'G';
    v[0xe] = 'F';
    v[0xf] = 'z';
    v[0x10] = 'X';
    v[0x11] = 'Y';
    v[0x12] = ']';
    v[0x13] = '_';
    v[0x14] = 'I';
    v[0x15] = '\x18';
    v[0x16] = 'n';
    v[0x17] = 'G';
    v[0x18] = 'L';
    v[0x19] = '4';
    v[0x1a] = '$';
    v[0x1b] = '/';
    v[0x1c] = '\x00';
    v[0x1d] = '(';
    v[0x1e] = '$';
    v[0x1f] = '5';
    v[0x20] = '4';
    v[0x21] = '\x04';
    v[0x22] = '&';
    v[0x23] = '+';
    v[0x24] = '/';
    v[0x25] = ')';
    v[0x26] = '?';
    for (int i = 0; i < 0x27; ++i) {
        v[i] ^= (i + 0x27);
    }
    v[0x27] = '\0';
}

static inline void fill_classLoader(char v[]) {
    // Ljava/lang/ClassLoader;
    v[0x0] = '[';
    v[0x1] = 'r';
    v[0x2] = 'x';
    v[0x3] = 'l';
    v[0x4] = 'z';
    v[0x5] = '3';
    v[0x6] = 'q';
    v[0x7] = '\x7f';
    v[0x8] = 'q';
    v[0x9] = 'G';
    v[0xa] = '\x0e';
    v[0xb] = 'a';
    v[0xc] = 'O';
    v[0xd] = 'E';
    v[0xe] = 'V';
    v[0xf] = 'U';
    v[0x10] = 'k';
    v[0x11] = 'G';
    v[0x12] = 'H';
    v[0x13] = 'N';
    v[0x14] = 'N';
    v[0x15] = '^';
    v[0x16] = '\x16';
    for (int i = 0; i < 0x17; ++i) {
        v[i] ^= (i + 0x17);
    }
    v[0x17] = '\0';
}

static inline void fill_VMClassLoader(char v[]) {
    // java/lang/VMClassLoader
    v[0x0] = '}';
    v[0x1] = 'y';
    v[0x2] = 'o';
    v[0x3] = '{';
    v[0x4] = '4';
    v[0x5] = 'p';
    v[0x6] = '|';
    v[0x7] = 'p';
    v[0x8] = 'x';
    v[0x9] = '\x0f';
    v[0xa] = 'w';
    v[0xb] = 'o';
    v[0xc] = '`';
    v[0xd] = 'H';
    v[0xe] = 'D';
    v[0xf] = 'U';
    v[0x10] = 'T';
    v[0x11] = 'd';
    v[0x12] = 'F';
    v[0x13] = 'K';
    v[0x14] = 'O';
    v[0x15] = 'I';
    v[0x16] = '_';
    for (int i = 0; i < 0x17; ++i) {
        v[i] ^= (i + 0x17);
    }
    v[0x17] = '\0';
}

static inline void fill_XposedBridge(char v[]) {
    // de/robv/android/xposed/XposedBridge
    v[0x0] = 'G';
    v[0x1] = 'A';
    v[0x2] = '\n';
    v[0x3] = 'T';
    v[0x4] = 'H';
    v[0x5] = 'J';
    v[0x6] = '_';
    v[0x7] = '\x05';
    v[0x8] = 'J';
    v[0x9] = 'B';
    v[0xa] = 'I';
    v[0xb] = '\\';
    v[0xc] = '@';
    v[0xd] = 'Y';
    v[0xe] = 'U';
    v[0xf] = '\x1d';
    v[0x10] = 'K';
    v[0x11] = 'D';
    v[0x12] = 'Z';
    v[0x13] = 'E';
    v[0x14] = 'R';
    v[0x15] = '\\';
    v[0x16] = '\x16';
    v[0x17] = 'b';
    v[0x18] = 'K';
    v[0x19] = 'S';
    v[0x1a] = 'N';
    v[0x1b] = '[';
    v[0x1c] = '[';
    v[0x1d] = '\x02';
    v[0x1e] = '3';
    v[0x1f] = '+';
    v[0x20] = '\'';
    v[0x21] = '#';
    v[0x22] = ' ';
    for (int i = 0; i < 0x23; ++i) {
        v[i] ^= (i + 0x23);
    }
    v[0x23] = '\0';
}

static inline void fill_findLoadedClass_signature(char v[]) {
    // (Ljava/lang/ClassLoader;Ljava/lang/String;)Ljava/lang/Class;
    v[0x0] = '\x14';
    v[0x1] = 'q';
    v[0x2] = 'T';
    v[0x3] = '^';
    v[0x4] = '6';
    v[0x5] = ' ';
    v[0x6] = 'm';
    v[0x7] = '/';
    v[0x8] = '%';
    v[0x9] = '+';
    v[0xa] = '!';
    v[0xb] = 'h';
    v[0xc] = '\x0b';
    v[0xd] = '%';
    v[0xe] = '+';
    v[0xf] = '8';
    v[0x10] = '?';
    v[0x11] = '\x01';
    v[0x12] = '!';
    v[0x13] = '.';
    v[0x14] = '4';
    v[0x15] = '4';
    v[0x16] = ' ';
    v[0x17] = 'h';
    v[0x18] = '\x18';
    v[0x19] = '?';
    v[0x1a] = '7';
    v[0x1b] = '!';
    v[0x1c] = '9';
    v[0x1d] = 'v';
    v[0x1e] = '6';
    v[0x1f] = ':';
    v[0x20] = '2';
    v[0x21] = ':';
    v[0x22] = 'q';
    v[0x23] = '\x0c';
    v[0x24] = '\x14';
    v[0x25] = '\x13';
    v[0x26] = '\x0b';
    v[0x27] = '\r';
    v[0x28] = '\x03';
    v[0x29] = '^';
    v[0x2a] = 'O';
    v[0x2b] = '+';
    v[0x2c] = '\x02';
    v[0x2d] = '\x08';
    v[0x2e] = '\x1c';
    v[0x2f] = '\n';
    v[0x30] = 'C';
    v[0x31] = '\x01';
    v[0x32] = '\x0f';
    v[0x33] = '\x01';
    v[0x34] = '\x17';
    v[0x35] = '^';
    v[0x36] = '1';
    v[0x37] = '\x1f';
    v[0x38] = '\x15';
    v[0x39] = '\x06';
    v[0x3a] = '\x05';
    v[0x3b] = 'L';
    for (int i = 0; i < 0x3c; ++i) {
        v[i] ^= (i + 0x3c);
    }
    v[0x3c] = '\0';
}

static inline void file_xposed_native_signature(char v[]) {
    // (Ljava/lang/reflect/Member;I[Ljava/lang/Class;Ljava/lang/Class;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;
    v[0x0] = ';';
    v[0x1] = 'L';
    v[0x2] = 'k';
    v[0x3] = 'c';
    v[0x4] = 'u';
    v[0x5] = 'e';
    v[0x6] = '*';
    v[0x7] = 'j';
    v[0x8] = 'f';
    v[0x9] = 'f';
    v[0xa] = 'n';
    v[0xb] = '%';
    v[0xc] = 'y';
    v[0xd] = 'i';
    v[0xe] = 'k';
    v[0xf] = 'b';
    v[0x10] = 'j';
    v[0x11] = 's';
    v[0x12] = 'e';
    v[0x13] = '=';
    v[0x14] = '^';
    v[0x15] = 'e';
    v[0x16] = 'l';
    v[0x17] = '`';
    v[0x18] = 'f';
    v[0x19] = 'v';
    v[0x1a] = '>';
    v[0x1b] = 'O';
    v[0x1c] = '\\';
    v[0x1d] = 'D';
    v[0x1e] = 'c';
    v[0x1f] = 'k';
    v[0x20] = '}';
    v[0x21] = 'm';
    v[0x22] = '"';
    v[0x23] = 'b';
    v[0x24] = 'n';
    v[0x25] = '~';
    v[0x26] = 'v';
    v[0x27] = '=';
    v[0x28] = 'P';
    v[0x29] = 'l';
    v[0x2a] = '`';
    v[0x2b] = 'q';
    v[0x2c] = 'p';
    v[0x2d] = '?';
    v[0x2e] = 'I';
    v[0x2f] = 'l';
    v[0x30] = 'f';
    v[0x31] = '~';
    v[0x32] = 'h';
    v[0x33] = '%';
    v[0x34] = 'g';
    v[0x35] = 'm';
    v[0x36] = 'c';
    v[0x37] = 'i';
    v[0x38] = ' ';
    v[0x39] = 'S';
    v[0x3a] = '}';
    v[0x3b] = 's';
    v[0x3c] = '`';
    v[0x3d] = 's';
    v[0x3e] = ':';
    v[0x3f] = 'N';
    v[0x40] = 'i';
    v[0x41] = 'e';
    v[0x42] = 's';
    v[0x43] = 'g';
    v[0x44] = '(';
    v[0x45] = 'd';
    v[0x46] = 'h';
    v[0x47] = 'd';
    v[0x48] = 'l';
    v[0x49] = '#';
    v[0x4a] = 'B';
    v[0x4b] = 'l';
    v[0x4c] = 'e';
    v[0x4d] = 'u';
    v[0x4e] = 'r';
    v[0x4f] = 'f';
    v[0x50] = '(';
    v[0x51] = '[';
    v[0x52] = 'M';
    v[0x53] = 'h';
    v[0x54] = 'b';
    v[0x55] = 'r';
    v[0x56] = 'd';
    v[0x57] = ')';
    v[0x58] = 'k';
    v[0x59] = 'i';
    v[0x5a] = 'g';
    v[0x5b] = 'm';
    v[0x5c] = '$';
    v[0x5d] = 'C';
    v[0x5e] = 'o';
    v[0x5f] = 'd';
    v[0x60] = 'j';
    v[0x61] = 's';
    v[0x62] = 'e';
    v[0x63] = ')';
    v[0x64] = ':';
    v[0x65] = 'L';
    v[0x66] = 'k';
    v[0x67] = 'c';
    v[0x68] = 'u';
    v[0x69] = 'e';
    v[0x6a] = '*';
    v[0x6b] = 'j';
    v[0x6c] = 'f';
    v[0x6d] = 'f';
    v[0x6e] = 'n';
    v[0x6f] = '%';
    v[0x70] = 'D';
    v[0x71] = 'n';
    v[0x72] = 'g';
    v[0x73] = 'k';
    v[0x74] = 'l';
    v[0x75] = 'd';
    v[0x76] = '*';
    for (int i = 0; i < 0x77; ++i) {
        v[i] ^= ((i + 0x77) % 20);
    }
    v[0x77] = '\0';
}

static inline void fill_xposed_invoke_signature(char v[]) {
    // (Ljava/lang/reflect/Member;ILjava/lang/Object;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;
    v[0x0] = '*';
    v[0x1] = 'O';
    v[0x2] = 'n';
    v[0x3] = 'd';
    v[0x4] = 'p';
    v[0x5] = 'f';
    v[0x6] = '\'';
    v[0x7] = 'e';
    v[0x8] = 'k';
    v[0x9] = 'e';
    v[0xa] = 'k';
    v[0xb] = '"';
    v[0xc] = '|';
    v[0xd] = 'j';
    v[0xe] = 'v';
    v[0xf] = '}';
    v[0x10] = 'w';
    v[0x11] = 'p';
    v[0x12] = 't';
    v[0x13] = '.';
    v[0x14] = 'O';
    v[0x15] = 'f';
    v[0x16] = 'i';
    v[0x17] = 'g';
    v[0x18] = 'c';
    v[0x19] = 'u';
    v[0x1a] = '3';
    v[0x1b] = '@';
    v[0x1c] = 'F';
    v[0x1d] = 'a';
    v[0x1e] = 'm';
    v[0x1f] = '{';
    v[0x20] = 'o';
    v[0x21] = ' ';
    v[0x22] = '|';
    v[0x23] = 'p';
    v[0x24] = '|';
    v[0x25] = 't';
    v[0x26] = '/';
    v[0x27] = 'N';
    v[0x28] = '`';
    v[0x29] = 'i';
    v[0x2a] = 'a';
    v[0x2b] = 'f';
    v[0x2c] = 'r';
    v[0x2d] = '<';
    v[0x2e] = 'D';
    v[0x2f] = 'c';
    v[0x30] = 'k';
    v[0x31] = '}';
    v[0x32] = 'm';
    v[0x33] = '"';
    v[0x34] = 'b';
    v[0x35] = 'n';
    v[0x36] = '~';
    v[0x37] = 'v';
    v[0x38] = '=';
    v[0x39] = '\\';
    v[0x3a] = 'b';
    v[0x3b] = 'k';
    v[0x3c] = 'g';
    v[0x3d] = '`';
    v[0x3e] = 'p';
    v[0x3f] = '>';
    v[0x40] = ']';
    v[0x41] = 'K';
    v[0x42] = 'b';
    v[0x43] = 'h';
    v[0x44] = '|';
    v[0x45] = 'j';
    v[0x46] = '#';
    v[0x47] = 'a';
    v[0x48] = 'o';
    v[0x49] = 'a';
    v[0x4a] = 'w';
    v[0x4b] = '>';
    v[0x4c] = ']';
    v[0x4d] = 'q';
    v[0x4e] = 'j';
    v[0x4f] = 'd';
    v[0x50] = 'a';
    v[0x51] = 'w';
    v[0x52] = '?';
    v[0x53] = ',';
    v[0x54] = 'J';
    v[0x55] = 'm';
    v[0x56] = 'i';
    v[0x57] = '\x7f';
    v[0x58] = 'k';
    v[0x59] = '$';
    v[0x5a] = '`';
    v[0x5b] = 'l';
    v[0x5c] = '`';
    v[0x5d] = 'h';
    v[0x5e] = '?';
    v[0x5f] = '^';
    v[0x60] = 'p';
    v[0x61] = 'y';
    v[0x62] = 'e';
    v[0x63] = 'b';
    v[0x64] = 'v';
    v[0x65] = '8';
    for (int i = 0; i < 0x66; ++i) {
        v[i] ^= ((i + 0x66) % 20);
    }
    v[0x66] = '\0';
}

static inline void fill_xposed_callback_method_l(char v[]) {
    // _ZN3art6mirror9ArtMethod22xposed_callback_methodE
    v[0x0] = 'V';
    v[0x1] = 'P';
    v[0x2] = 'E';
    v[0x3] = '?';
    v[0x4] = 'l';
    v[0x5] = '|';
    v[0x6] = '{';
    v[0x7] = '&';
    v[0x8] = '|';
    v[0x9] = '{';
    v[0xa] = 'a';
    v[0xb] = 'r';
    v[0xc] = 'n';
    v[0xd] = 'p';
    v[0xe] = ':';
    v[0xf] = 'E';
    v[0x10] = 'w';
    v[0x11] = 'r';
    v[0x12] = 'J';
    v[0x13] = 'm';
    v[0x14] = '}';
    v[0x15] = 'b';
    v[0x16] = 'd';
    v[0x17] = 'h';
    v[0x18] = '?';
    v[0x19] = '<';
    v[0x1a] = 'w';
    v[0x1b] = '`';
    v[0x1c] = '~';
    v[0x1d] = 'a';
    v[0x1e] = 'v';
    v[0x1f] = 'd';
    v[0x20] = '^';
    v[0x21] = 'a';
    v[0x22] = 'b';
    v[0x23] = 'h';
    v[0x24] = 'i';
    v[0x25] = 'd';
    v[0x26] = 'f';
    v[0x27] = 'k';
    v[0x28] = 'b';
    v[0x29] = 'U';
    v[0x2a] = 'f';
    v[0x2b] = 'i';
    v[0x2c] = 'y';
    v[0x2d] = 'f';
    v[0x2e] = '`';
    v[0x2f] = 't';
    v[0x30] = 'T';
    for (int i = 0; i < 0x31; ++i) {
        v[i] ^= ((i + 0x31) % 20);
    }
    v[0x31] = '\0';
}

static inline void fill_xposed_callback_method_m(char v[]) {
    // _ZN3art9ArtMethod22xposed_callback_methodE
    v[0x0] = ']';
    v[0x1] = 'Y';
    v[0x2] = 'J';
    v[0x3] = '6';
    v[0x4] = 'g';
    v[0x5] = 'u';
    v[0x6] = '|';
    v[0x7] = '0';
    v[0x8] = 'K';
    v[0x9] = 'y';
    v[0xa] = 'x';
    v[0xb] = '@';
    v[0xc] = 'k';
    v[0xd] = '{';
    v[0xe] = 'x';
    v[0xf] = '~';
    v[0x10] = 'v';
    v[0x11] = '!';
    v[0x12] = '2';
    v[0x13] = 'y';
    v[0x14] = 'r';
    v[0x15] = 'l';
    v[0x16] = 'w';
    v[0x17] = '`';
    v[0x18] = 'b';
    v[0x19] = 'X';
    v[0x1a] = 'k';
    v[0x1b] = 'h';
    v[0x1c] = 'f';
    v[0x1d] = 'g';
    v[0x1e] = 'n';
    v[0x1f] = 'l';
    v[0x20] = 'm';
    v[0x21] = 'd';
    v[0x22] = 'O';
    v[0x23] = '|';
    v[0x24] = 'w';
    v[0x25] = 'g';
    v[0x26] = 'h';
    v[0x27] = 'n';
    v[0x28] = 'f';
    v[0x29] = 'F';
    for (int i = 0; i < 0x2a; ++i) {
        v[i] ^= ((i + 0x2a) % 20);
    }
    v[0x2a] = '\0';
}

static inline void fill_loader(char v[]) {
    // loader
    v[0x0] = 'j';
    v[0x1] = 'h';
    v[0x2] = 'i';
    v[0x3] = 'm';
    v[0x4] = 'o';
    v[0x5] = 'y';
    for (int i = 0; i < 0x6; ++i) {
        v[i] ^= ((i + 0x6) % 20);
    }
    v[0x6] = '\0';
}

static inline void fill_findLoadedClass(char v[]) {
    // findLoadedClass
    v[0x0] = 'i';
    v[0x1] = 'y';
    v[0x2] = '\x7f';
    v[0x3] = 'v';
    v[0x4] = '_';
    v[0x5] = 'o';
    v[0x6] = '`';
    v[0x7] = 'f';
    v[0x8] = 'f';
    v[0x9] = '`';
    v[0xa] = 'F';
    v[0xb] = 'j';
    v[0xc] = 'f';
    v[0xd] = '{';
    v[0xe] = 'z';
    for (int i = 0; i < 0xf; ++i) {
        v[i] ^= ((i + 0xf) % 20);
    }
    v[0xf] = '\0';
}

static inline void fill_invokeOriginalMethodNative(char v[]) {
    // invokeOriginalMethodNative
    v[0x0] = 'o';
    v[0x1] = 'i';
    v[0x2] = '~';
    v[0x3] = 'f';
    v[0x4] = 'a';
    v[0x5] = 'n';
    v[0x6] = 'C';
    v[0x7] = '\x7f';
    v[0x8] = 'g';
    v[0x9] = 'h';
    v[0xa] = 'y';
    v[0xb] = '\x7f';
    v[0xc] = 's';
    v[0xd] = '\x7f';
    v[0xe] = 'M';
    v[0xf] = 'd';
    v[0x10] = 'v';
    v[0x11] = 'k';
    v[0x12] = 'k';
    v[0x13] = 'a';
    v[0x14] = 'H';
    v[0x15] = 'f';
    v[0x16] = '|';
    v[0x17] = '`';
    v[0x18] = '|';
    v[0x19] = 'n';
    for (int i = 0; i < 0x1a; ++i) {
        v[i] ^= ((i + 0x1a) % 20);
    }
    v[0x1a] = '\0';
}

static inline void fill_invoke(char v[]) {
    // invoke
    v[0x0] = 'o';
    v[0x1] = 'i';
    v[0x2] = '~';
    v[0x3] = 'f';
    v[0x4] = 'a';
    v[0x5] = 'n';
    for (int i = 0; i < 0x6; ++i) {
        v[i] ^= ((i + 0x6) % 20);
    }
    v[0x6] = '\0';
}

static inline void fill_handleHookedMethod(char v[]) {
    // handleHookedMethod
    v[0x0] = 'z';
    v[0x1] = 'r';
    v[0x2] = 'n';
    v[0x3] = 'e';
    v[0x4] = 'n';
    v[0x5] = 'f';
    v[0x6] = 'L';
    v[0x7] = 'j';
    v[0x8] = 'i';
    v[0x9] = 'l';
    v[0xa] = 'm';
    v[0xb] = 'm';
    v[0xc] = 'G';
    v[0xd] = 'n';
    v[0xe] = 'x';
    v[0xf] = 'e';
    v[0x10] = 'a';
    v[0x11] = 'k';
    for (int i = 0; i < 0x12; ++i) {
        v[i] ^= ((i + 0x12) % 20);
    }
    v[0x12] = '\0';
}

static inline void fill_libartso(char v[]) {
    // libart.so
    v[0x0] = 'e';
    v[0x1] = 'c';
    v[0x2] = 'i';
    v[0x3] = 'm';
    v[0x4] = '\x7f';
    v[0x5] = 'z';
    v[0x6] = '!';
    v[0x7] = 'c';
    v[0x8] = '~';
    for (int i = 0; i < 0x9; ++i) {
        v[i] ^= ((i + 0x9) % 20);
    }
    v[0x9] = '\0';
}

static jboolean antiXposed(JNIEnv *env, jclass clazz) {
    jboolean result = JNI_FALSE;
    // current max is 0x77
    char v1[0x80], v2[0x80];

    if (!checkXposed()) {
        return JNI_TRUE;
    }

    xposed = true;
    // FIXME: log "try disable xposed hooks"

    fill_classLoader$SystemClassLoader(v1);
    jclass classLoader$SystemClassLoader = env->FindClass(v1);
    fill_classLoader(v1);
    fill_loader(v2);
    jobject systemClassLoader = env->GetStaticObjectField(classLoader$SystemClassLoader,
        env->GetStaticFieldID(classLoader$SystemClassLoader, v2, v1));

    fill_VMClassLoader(v1);
    jclass vmClassLoader = env->FindClass(v1);
    fill_XposedBridge(v1);
    jstring stringXposedBridge = env->NewStringUTF(v1);
    fill_findLoadedClass_signature(v1);
    fill_findLoadedClass(v2);
    jclass classXposedBridge = static_cast<jclass>(env->CallStaticObjectMethod(vmClassLoader,
        env->GetStaticMethodID(vmClassLoader, v2, v1),
        systemClassLoader, stringXposedBridge));

    file_xposed_native_signature(v1);
    fill_invokeOriginalMethodNative(v2);
    original = env->GetStaticMethodID(classXposedBridge, v2, v1);

    fill_xposed_invoke_signature(v1);
    fill_invoke(v2);
    jmethodID replace = env->GetStaticMethodID(clazz, v2, v1);

    fill_handleHookedMethod(v2);
    jmethodID hooked = env->GetStaticMethodID(classXposedBridge, v2, v1);

    jmethodID *xposedCallbackMethod = NULL;
    fill_libartso(v2);
    void *handle = dlopen(v2, RTLD_NOW);
    if (handle != NULL) {
        if (sdk() < 23) {
            fill_xposed_callback_method_l(v1);
        } else {
            fill_xposed_callback_method_m(v1);
        }
        xposedCallbackMethod = reinterpret_cast<jmethodID *>(dlsym(handle, v1));
        dlclose(handle);
        if (xposedCallbackMethod != NULL && *xposedCallbackMethod == hooked) {
            *xposedCallbackMethod = replace;
            result = JNI_TRUE;
            goto clean;
        }
    }

    xposedCallbackMethod = checkCallback(hooked);
    if (xposedCallbackMethod != NULL) {
        *xposedCallbackMethod = replace;
        result = JNI_TRUE;
        goto clean;
    }

    // FIXME: log "cannot disable xposed hooks"
    // FIXME: log "add libart.so to /system/etc/public.libraries.txt"

clean:
    env->DeleteLocalRef(classXposedBridge);
    env->DeleteLocalRef(stringXposedBridge);
    env->DeleteLocalRef(vmClassLoader);
    env->DeleteLocalRef(systemClassLoader);
    env->DeleteLocalRef(classLoader$SystemClassLoader);

    return result;
}

#define UNSIGNED(x) static_cast<unsigned>(x[0] | (x[1] << 8) | (x[2] << 16) | (x[3] << 24))

static int checkSignature(const char *path) {
    unsigned char buffer[0x10];
    unsigned offset, size;
    int hash;

#ifdef DEBUG
    LOGI("check signature for %s", path);
#endif
    int ret = 1;
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        return 1;
    }

    // for comment length
    lseek(fd, -0x2, SEEK_END);
    read(fd, buffer, 0x2);
    if (buffer[0x0] != 0 || buffer[0x1] != 0) {
        goto clean;
    }

    // for offset
    lseek(fd, -0x6, SEEK_CUR);
    read(fd, buffer, 0x4);
    offset = UNSIGNED(buffer);

    lseek(fd, static_cast<off_t>(offset - 0x18), SEEK_SET);
    read(fd, buffer, 0x8);
    size = UNSIGNED(buffer);

    read(fd, buffer, 0x10);
    for (int i = 0; i < 0x10; ++i) {
        buffer[i] ^= (i + 0x11);
    }
    if (memcmp(buffer, "PBX4F\x7fp8[vt\x7fv>+\x12", 0x10) != 0) {
        goto clean;
    }

    lseek(fd, static_cast<off_t>(offset - (size + 0x8)), SEEK_SET);
    read(fd, buffer, 0x8);
    if (UNSIGNED(buffer) != size) {
        goto clean;
    }

    for (;;) {
        read(fd, buffer, 0x8);
        size = UNSIGNED(buffer);

        read(fd, buffer, 0x4);
        if ((UNSIGNED(buffer) ^ 0x0d0c0b0a) == 0x7c058c10) {
            read(fd, buffer, 0x4); // signer-sequence length
            read(fd, buffer, 0x4); // signer length
            read(fd, buffer, 0x4); // signed data length

            read(fd, buffer, 0x4); // digests-sequence length
            size = UNSIGNED(buffer);
            lseek(fd, static_cast<off_t>(size), SEEK_CUR);// skip digests

            read(fd, buffer, 0x4); // certificates length
            read(fd, buffer, 0x4); // certificate length
            size = UNSIGNED(buffer);
            if (size == GENUINE_SIZE) {
                hash = 1;
                for (unsigned i = 0; i < size; ++i) {
                    read(fd, buffer, 0x1);
                    hash = 31 * hash + static_cast<signed char>(buffer[0]);
                }
                if ((static_cast<unsigned>(hash) ^ 0x14131211) == GENUINE_HASH) {
                    ret = 0;
                }
            }
            break;
        } else {
            lseek(fd, static_cast<off_t>(size - 0x4), SEEK_CUR);
        }
    }

clean:
    close(fd);

    return ret;
}

static void rstrip(char *loader) {
    char *path;
    path = strchr(loader, '\r');
    if (path != NULL) {
        *path = '\0';
    }
    path = strchr(loader, '\n');
    if (path != NULL) {
        *path = '\0';
    }
}

static inline bool isapk(const char *str) {
    const char *dot = strrchr(str, '.');
    return dot != NULL
           && *++dot == 'a'
           && *++dot == 'p'
           && *++dot == 'k'
           && (*++dot == '\0' || *dot == '\r' || *dot == '\n');
}

#ifdef ANTI_ODEX
static inline bool isdex(const char *str) {
    const char *dot = strrchr(str, '.');
    return dot != NULL
           && *++dot == 'd'
           && *++dot == 'e'
           && *++dot == 'x'
           && (*++dot == '\0' || *dot == '\r' || *dot == '\n');
}

static inline bool isodex(const char *str) {
    const char *dot = strrchr(str, '.');
    return dot != NULL
           && *++dot == 'o'
           && *++dot == 'd'
           && *++dot == 'e'
           && *++dot == 'x'
           && (*++dot == '\0' || *dot == '\r' || *dot == '\n');
}

static inline size_t fill_dex2oat_cmdline(char v[]) {
    // dex2oat-cmdline
    v[0x0] = 'k';
    v[0x1] = 'u';
    v[0x2] = 'i';
    v[0x3] = ' ';
    v[0x4] = '|';
    v[0x5] = 'a';
    v[0x6] = 'u';
    v[0x7] = '/';
    v[0x8] = '`';
    v[0x9] = 'i';
    v[0xa] = 'a';
    v[0xb] = 'j';
    v[0xc] = 'n';
    v[0xd] = 'f';
    v[0xe] = 'l';
    for (int i = 0; i < 0xf; ++i) {
        v[i] ^= ((i + 0xf) % 20);
    }
    v[0xf] = '\0';
    return 0xf;
}

static inline size_t fill_dex_file(char v[]) {
    // --dex-file
    v[0x0] = '\'';
    v[0x1] = '&';
    v[0x2] = 'h';
    v[0x3] = 'h';
    v[0x4] = 'v';
    v[0x5] = '"';
    v[0x6] = 'v';
    v[0x7] = 'x';
    v[0x8] = '~';
    v[0x9] = 'v';
    for (int i = 0; i < 0xa; ++i) {
        v[i] ^= ((i + 0xa) % 20);
    }
    v[0xa] = '\0';
    return 0xa;
}

static int checkOdex(const char *path) {
    size_t len;
    char *cmdline;
    char buffer[0x400], find[64];

    int ret = 0;
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        return 1;
    }

    lseek(fd, 0x1000, SEEK_SET);
    read(fd, buffer, 0x400);

    cmdline = buffer;
    len = fill_dex2oat_cmdline(find) + 1;
    for (int i = 0; i < 0x200; ++i, ++cmdline) {
        if (memcmp(cmdline, find, len) == 0) {
            cmdline += len;
            fill_dex_file(find);
            if ((ret = (strstr(cmdline, find) != NULL))) {
                fill_dex2oat_cmdline(find);
                LOGE("%s: %s", find, cmdline);
            }
            break;
        }
    }

    close(fd);

    return ret;
}
#endif

static inline void fix_name(char name[], int length) {
    for (int i = 0; i < length; ++i) {
        name[i] ^= ((i + length) % 20);
    }
    name[length] = '\0';
}

static inline bool isSameFile(char *path1, char *path2) {
    struct stat stat1, stat2;
    if (lstat(path1, &stat1)) {
        LOGE("%s: %s", path1, strerror(errno));
        return false;
    }
    if (lstat(path2, &stat2)) {
        LOGE("%s: %s", path2, strerror(errno));
        return false;
    }
    return stat1.st_dev == stat2.st_dev && stat1.st_ino == stat2.st_ino;
}

static inline void fill_classes_dex(char dex[]) {
    dex[0x0] = 'h';
    dex[0x1] = '`';
    dex[0x2] = 'l';
    dex[0x3] = '}';
    dex[0x4] = '|';
    dex[0x5] = 'u';
    dex[0x6] = 'b';
    dex[0x7] = '<';
    dex[0x8] = 'w';
    dex[0x9] = 'e';
    dex[0xa] = 'y';
    for (int i = 0; i < 0xb; ++i) {
        dex[i] ^= ((i + 0xb) % 20);
    }
    dex[0xb] = '\0';
}

static inline bool hasDex(char *path) {
    unsigned char buffer[0x100];
    unsigned offset, size;
    char dex[0xb];
    bool result = false;

#ifdef DEBUG
    LOGI("check classes.dex for %s", path);
#endif
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
#ifdef DEBUG
        LOGW("cannot open %s", path);
#endif
        return true;
    }

    for (int i = 0; i < 0x10000; ++i) {
        lseek(fd, -i - 2, SEEK_END);
        read(fd, buffer, 0x2);
        size = static_cast<unsigned>(buffer[0]) | static_cast<unsigned>(buffer[1]) << 8;
        if (size == static_cast<unsigned>(i)) {
            lseek(fd, -22, SEEK_CUR);
            read(fd, buffer, 0x4);
            size = UNSIGNED(buffer);
            if (size == 0x06054b50) {
                lseek(fd, 12, SEEK_CUR);
                break;
            }
        }
        if (i == 0xffff) {
#ifdef DEBUG
            LOGW("cannot found eocd in %s", path);
#endif
            result = true;
            goto clean;
        }
    }

    read(fd, buffer, 0x4);
    offset = UNSIGNED(buffer);

    lseek(fd, offset, SEEK_SET);
    fill_classes_dex(dex);
    for (;;) {
        read(fd, buffer, 0x4);
        size = UNSIGNED(buffer);
        if (size != 0x02014b50) {
            break;
        }
        lseek(fd, 0x18, SEEK_CUR);
        // File name length (n)
        read(fd, buffer, 0x2);
        size = static_cast<unsigned>(buffer[0]) | static_cast<unsigned>(buffer[1]) << 8;
        // Extra field length (m)
        read(fd, buffer, 0x2);
        offset = static_cast<unsigned>(buffer[0]) | static_cast<unsigned>(buffer[1]) << 8;
        // File comment length (k)
        read(fd, buffer, 0x2);
        offset += static_cast<unsigned>(buffer[0]) | static_cast<unsigned>(buffer[1]) << 8;
        lseek(fd, 0xc, SEEK_CUR);
        read(fd, buffer, size);
        buffer[size] = '\0';
#if 0
        LOGD("%s", buffer);
#endif
        if (memcmp(buffer, dex, 11) == 0) {
#ifdef DEBUG
            LOGI("found %s in %s", buffer, path);
#endif
            result = true;
            break;
        }
        lseek(fd, offset, SEEK_CUR);
    }

clean:
    close(fd);

    return result;
}

static int checkGenuine() {
    FILE *fp;
    char maps[16] = {0};
    char apk[NAME_MAX];
    char line[PATH_MAX];
    char name[] = GENUINE_NAME;
    int check = CHECK_UNKNOWN;
    char *data = getenv("ANDROID_DATA");

    memset(apk, 0, NAME_MAX);
    fix_name(name, (sizeof(name) / sizeof(char)) - 1);
    fill_proc_self_maps(maps);
    fp = fopen(maps, "r");
    if (fp == NULL) {
        return check;
    }

    while (fgets(line, PATH_MAX - 1, fp) != NULL) {
        char *path = line;
        if (strchr(line, '/') == NULL) {
            continue;
        }
        while (*path != '/') {
            ++path;
        }
        rstrip(path);
#ifdef ANTI_ODEX
        if ((isodex(path) || isdex(path))) {
#ifdef DEBUG
            LOGI("check %s", path);
#endif
            if (strstr(path, name) != NULL && access(path, F_OK) == 0 && checkOdex(path)) {
                LOGE("%s", path);
                check = JNI_FALSE;
                break;
            }
        }
#endif
        if (isapk(path)) {
#ifdef DEBUG
            LOGI("check %s", path);
#endif
            if (strstr(path, name) != NULL && access(path, F_OK) == 0) {
                if (apk[0] != 0) {
                    if (!strcmp(path, apk) || isSameFile(path, apk)) {
                        check = CHECK_TRUE;
                    } else {
                        LOGE("%s != %s", path, apk);
                        check = CHECK_FALSE;
                        goto clean;
                    }
                } else {
                    if (checkSignature(path)) {
                        LOGE("%s", path);
                        check = CHECK_FALSE;
                        goto clean;
                    } else {
#ifdef DEBUG
                        LOGI("%s", path);
#endif
                        check = CHECK_TRUE;
                        sprintf(apk, "%s", path);
                    }
                }
            } else if (strncmp(path, data, strlen(data)) == 0) {
                if (!xposed && hasDex(path)) {
                    LOGW("%s", path);
                    check = CHECK_ERROR;
                    goto clean;
                }
#ifdef ANTI_OVERLAY
                LOGE("%s", path);
                check = CHECK_FALSE;
                goto clean;
#else
                if (check == CHECK_UNKNOWN) {
                    LOGW("%s", path);
                    check = CHECK_FALSE;
                }
#endif
            }
        }
    }

    if (check == CHECK_UNKNOWN) {
        check = CHECK_TRUE;
    }

clean:
    fclose(fp);

    return check;
}

#ifndef NELEM
#define NELEM(x) static_cast<int>(sizeof(x) / sizeof((x)[0]))
#endif

// invoke length is 7
static char xposedInvokeName[0x10];
// signature length is 0x66
static char xposedInvokeSignature[0x80];

// FIXME: define methods in your own class
// FIXME: private static native Object invoke(Member m, int i, Object a, Object t, Object[] as) throws Throwable;
// FIXME: public static native int version();

#ifndef GENUINE_CLAZZ
#define GENUINE_CLAZZ "me/piebridge/Genuine"
#endif

static JNINativeMethod methods[] = {
        {xposedInvokeName, xposedInvokeSignature, reinterpret_cast<void *>(invoke)},
        {"version",        "()I",                 reinterpret_cast<void *>(version)},
};

#ifndef JNI_ONLOAD
jint JNI_OnLoad(JavaVM *jvm, void *) {
    JNIEnv *env;
    jclass clazz;

#ifdef DEBUG
    LOGI("JNI_OnLoad start");
#endif

    if (jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    if ((clazz = env->FindClass(GENUINE_CLAZZ)) == NULL) {
        return JNI_ERR;
    }

    fill_invoke(const_cast<char *>( methods[0].name));
    fill_xposed_invoke_signature(const_cast<char *>(methods[0].signature));
    if (env->RegisterNatives(clazz, methods, NELEM(methods)) < 0) {
        return JNI_ERR;
    }

#ifdef DEBUG
    LOGI("JNI_OnLoad_Extra start");
#endif
    if (JNI_OnLoad_Extra(env, clazz) < 0) {
        return JNI_ERR;
    }

#ifdef DEBUG
    LOGI("antiXposed start");
#endif
    if (!antiXposed(env, clazz)) {
        return JNI_ERR;
    }

#ifdef DEBUG
    LOGI("checkGenuine start");
#endif
    genuine = checkGenuine();

    env->DeleteLocalRef(clazz);

#ifdef DEBUG
    LOGI("JNI_OnLoad end, genuine: %d", genuine);
#endif

    if (genuine == CHECK_ERROR) {
#ifdef DEBUG
        LOGE("JNI_ERR");
#endif
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}
#endif
