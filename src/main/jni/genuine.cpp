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
#include "plt.h"

#ifdef SUPPORT_EPIC
#include "epic.h"
#endif

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

__attribute__ ((visibility ("internal")))
int sdk;

__attribute__ ((visibility ("internal")))
bool throwOnError = true;

static bool xposed = false;

#ifdef SUPPORT_EPIC
static bool epic = false;
#endif

#ifdef ANTI_EDXPOSED
static bool edXposed = false;
#endif

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

    Symbol symbol;
    if (sdk < 23) {
        fill_xposed_callback_method_l(v1);
    } else {
        fill_xposed_callback_method_m(v1);
    }
    dl_iterate_phdr_symbol(&symbol, v1);

    jmethodID *xposedCallbackMethod = reinterpret_cast<jmethodID *>(symbol.symbol_sym);
#ifdef DEBUG
    LOGI("xposed_callback_method: %p", xposedCallbackMethod);
#endif
    if (xposedCallbackMethod != NULL && *xposedCallbackMethod == hooked) {
        *xposedCallbackMethod = replace;
        result = JNI_TRUE;
        goto clean;
    }

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

#ifdef ANTI_ODEX
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

static inline void fillDataApp(char map[]) {
    // /data/app
    map[0x0] = '&';
    map[0x1] = 'n';
    map[0x2] = 'j';
    map[0x3] = 'x';
    map[0x4] = 'l';
    map[0x5] = '!';
    map[0x6] = 'n';
    map[0x7] = '`';
    map[0x8] = 'a';
    for (int i = 0; i < 0x9; ++i) {
        map[i] ^= ((i + 0x9) % 20);
    }
    map[0x9] = '\0';
}


static inline bool isData(const char *path) {
    char v1[0x10];
    fillDataApp(v1);
    return strncmp(path, v1, strlen(v1)) == 0;
}

enum {
    TYPE_NON,
    TYPE_APK,
    TYPE_DEX,
};

static int checkGenuine() {
    FILE *fp;
    char maps[16] = {0};
    char apk[NAME_MAX];
    char line[PATH_MAX];
    char name[] = GENUINE_NAME;
    int check = CHECK_UNKNOWN;

    memset(apk, 0, NAME_MAX);
    fix_name(name, (sizeof(name) / sizeof(char)) - 1);
    fill_proc_self_maps(maps);
    fp = fopen(maps, "r");
    if (fp == NULL) {
        return CHECK_ERROR;
    }

    while (fgets(line, PATH_MAX - 1, fp) != NULL) {
        int type;
        char *path = line;
        if (strchr(line, '/') == NULL) {
            continue;
        }
        while (*path != '/') {
            ++path;
        }
        rstrip(path);
        if (isapk(path)) {
            type = TYPE_APK;
        } else if (isodex(path) || isdex(path)) {
            type = TYPE_DEX;
        } else {
            type = TYPE_NON;
        }
#ifdef ANTI_ODEX
        if (type == TYPE_DEX) {
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
        if (strstr(path, name) != NULL) {
            if (type == TYPE_APK && access(path, F_OK) == 0) {
#ifdef DEBUG
                LOGI("check %s", path);
#endif
                if (apk[0] != 0) {
                    if (!strcmp(path, apk) || isSameFile(path, apk)) {
                        check = CHECK_TRUE;
                    } else {
                        LOGE("%s != %s", path, apk);
                        check = CHECK_FALSE;
                    }
                } else {
                    if (checkSignature(path)) {
                        LOGE("%s", path);
                        check = CHECK_FALSE;
                    } else {
#ifdef DEBUG
                        LOGI("%s", path);
#endif
                        check = CHECK_TRUE;
                        sprintf(apk, "%s", path);
                    }
                }
            }
        } else if (type == TYPE_DEX && isData(path)) {
            bool allow;
            LOGW("%s", path);
            allow = xposed;
#ifdef SUPPORT_EPIC
            if (!allow) {
                allow = epic;
            }
#endif
#ifdef ANTI_EDXPOSED
            if (!allow) {
                allow = edXposed;
            }
#endif
            if (!allow) {
                check = CHECK_ERROR;
                goto clean;
            }
        } else if (type == TYPE_APK && isData(path)) {
            LOGW("%s", path);
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

    if (check == CHECK_UNKNOWN) {
        check = CHECK_TRUE;
    }

clean:
    fclose(fp);

    return check;
}

static inline void fillThread(char map[]) {
    // java/lang/Thread
    map[0x0] = 'z';
    map[0x1] = 'p';
    map[0x2] = 'd';
    map[0x3] = 'r';
    map[0x4] = '/';
    map[0x5] = 'm';
    map[0x6] = 'c';
    map[0x7] = 'm';
    map[0x8] = 'c';
    map[0x9] = '*';
    map[0xa] = 'R';
    map[0xb] = 'o';
    map[0xc] = 'z';
    map[0xd] = 'l';
    map[0xe] = 'k';
    map[0xf] = 'o';
    for (int i = 0; i < 0x10; ++i) {
        map[i] ^= ((i + 0x10) % 20);
    }
    map[0x10] = '\0';
}

static inline void fillSetUncaughtExceptionPreHandler(char map[]) {
    // setUncaughtExceptionPreHandler
    map[0x0] = 'y';
    map[0x1] = 'n';
    map[0x2] = 'x';
    map[0x3] = 'X';
    map[0x4] = '`';
    map[0x5] = 'l';
    map[0x6] = 'q';
    map[0x7] = 'd';
    map[0x8] = 'u';
    map[0x9] = '{';
    map[0xa] = 't';
    map[0xb] = 'D';
    map[0xc] = 'z';
    map[0xd] = '`';
    map[0xe] = 'a';
    map[0xf] = 'u';
    map[0x10] = 'r';
    map[0x11] = 'n';
    map[0x12] = 'g';
    map[0x13] = 'g';
    map[0x14] = 'Z';
    map[0x15] = 'y';
    map[0x16] = 'i';
    map[0x17] = 'E';
    map[0x18] = 'o';
    map[0x19] = 'a';
    map[0x1a] = 't';
    map[0x1b] = '}';
    map[0x1c] = 'w';
    map[0x1d] = 'a';
    for (int i = 0; i < 0x1e; ++i) {
        map[i] ^= ((i + 0x1e) % 20);
    }
    map[0x1e] = '\0';
}

static inline void fillSetUncaughtExceptionPreHandlerSignature(char map[]) {
    // (Ljava/lang/Thread$UncaughtExceptionHandler;)V
    map[0x0] = '.';
    map[0x1] = 'K';
    map[0x2] = 'b';
    map[0x3] = 'h';
    map[0x4] = '|';
    map[0x5] = 'j';
    map[0x6] = '#';
    map[0x7] = 'a';
    map[0x8] = 'o';
    map[0x9] = 'a';
    map[0xa] = 'w';
    map[0xb] = '>';
    map[0xc] = 'F';
    map[0xd] = '{';
    map[0xe] = 'r';
    map[0xf] = 'd';
    map[0x10] = 'c';
    map[0x11] = 'g';
    map[0x12] = ' ';
    map[0x13] = 'P';
    map[0x14] = 'h';
    map[0x15] = 'd';
    map[0x16] = 'i';
    map[0x17] = '|';
    map[0x18] = 'm';
    map[0x19] = 'c';
    map[0x1a] = 'x';
    map[0x1b] = 'H';
    map[0x1c] = 'v';
    map[0x1d] = 'l';
    map[0x1e] = 'u';
    map[0x1f] = 'a';
    map[0x20] = 'f';
    map[0x21] = 'z';
    map[0x22] = 'o';
    map[0x23] = 'o';
    map[0x24] = 'J';
    map[0x25] = 'b';
    map[0x26] = 'j';
    map[0x27] = 'a';
    map[0x28] = 'j';
    map[0x29] = 'b';
    map[0x2a] = 'z';
    map[0x2b] = '2';
    map[0x2c] = '#';
    map[0x2d] = ']';
    for (int i = 0; i < 0x2e; ++i) {
        map[i] ^= ((i + 0x2e) % 20);
    }
    map[0x2e] = '\0';
}

static void clearHandler(JNIEnv *env) {
    char v1[0x80], v2[0x80];
    fillThread(v1);
    jclass clazz = env->FindClass(v1);
    fillSetUncaughtExceptionPreHandler(v1);
    fillSetUncaughtExceptionPreHandlerSignature(v2);
    jmethodID method = env->GetStaticMethodID(clazz, v1, v2);
    if (method != NULL) {
        env->CallStaticVoidMethod(clazz, method, NULL);
    }
    env->DeleteLocalRef(clazz);
}

#if 0
static inline void debug(JNIEnv *env, const char *prefix, jobject object) {
    jclass classObject = env->FindClass("java/lang/Object");
    jmethodID objectToString = env->GetMethodID(classObject, "toString", "()Ljava/lang/String;");
    if (object == NULL) {
        LOGI(prefix, NULL);
    } else {
        jstring string = (jstring) env->CallObjectMethod(object, objectToString);
        const char *value = env->GetStringUTFChars(string, NULL);
        LOGI(prefix, value);
        env->ReleaseStringUTFChars(string, value);
        env->DeleteLocalRef(string);
    }
    env->DeleteLocalRef(classObject);
}
#endif

#ifdef ANTI_EDXPOSED
static inline void fillDisableHooks(char map[]) {
    // disableHooks
    map[0x0] = 'h';
    map[0x1] = 'd';
    map[0x2] = '}';
    map[0x3] = 'n';
    map[0x4] = 'r';
    map[0x5] = '}';
    map[0x6] = 'w';
    map[0x7] = '[';
    map[0x8] = 'o';
    map[0x9] = 'n';
    map[0xa] = 'i';
    map[0xb] = 'p';
    for (int i = 0; i < 0xc; ++i) {
        map[i] ^= ((i + 0xc) % 20);
    }
    map[0xc] = '\0';
}

static inline void fillZ(char map[]) {
    // Z
    map[0x0] = '[';
    for (int i = 0; i < 0x1; ++i) {
        map[i] ^= ((i + 0x1) % 20);
    }
    map[0x1] = '\0';
}

static bool doAntiEdXposed(JNIEnv *env, jobject classLoader) {
    char v1[0x80], v2[0x80];
    bool antied = false;

    fill_VMClassLoader(v1);
    jclass vmClassLoader = env->FindClass(v1);
    fill_XposedBridge(v1);
    jstring stringXposedBridge = env->NewStringUTF(v1);
    fill_findLoadedClass_signature(v1);
    fill_findLoadedClass(v2);
    jmethodID method = env->GetStaticMethodID(vmClassLoader, v2, v1);
    jclass classXposedBridge = (jclass) env->CallStaticObjectMethod(vmClassLoader, method,
                                                                    classLoader,
                                                                    stringXposedBridge);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }

    if (classXposedBridge != NULL) {
        fillDisableHooks(v1);
        fillZ(v2);
        jfieldID field = env->GetStaticFieldID(classXposedBridge, v1, v2);
        env->SetStaticBooleanField(classXposedBridge, field, JNI_TRUE);
        antied = true;
    }

    env->DeleteLocalRef(classXposedBridge);
    env->DeleteLocalRef(stringXposedBridge);
    env->DeleteLocalRef(vmClassLoader);

    return antied;
}

static inline void fillGInjectDexClassLoader(char map[]) {
    // gInjectDexClassLoader
    map[0x0] = 'f';
    map[0x1] = 'K';
    map[0x2] = 'm';
    map[0x3] = 'n';
    map[0x4] = '`';
    map[0x5] = 'e';
    map[0x6] = 's';
    map[0x7] = 'L';
    map[0x8] = 'l';
    map[0x9] = 'r';
    map[0xa] = 'H';
    map[0xb] = '`';
    map[0xc] = 'l';
    map[0xd] = '}';
    map[0xe] = '|';
    map[0xf] = '\\';
    map[0x10] = '~';
    map[0x11] = 's';
    map[0x12] = 'w';
    map[0x13] = 'e';
    map[0x14] = 's';
    for (int i = 0; i < 0x15; ++i) {
        map[i] ^= ((i + 0x15) % 20);
    }
    map[0x15] = '\0';
}

static bool antiEdXposed(JNIEnv *env) {
    bool antied = false;
    void *handle = dlopen(NULL, RTLD_NOW);
#ifdef DEBUG
    LOGI("handle: %p", handle);
#endif
    if (handle != NULL) {
        char v1[0x80];
        fillGInjectDexClassLoader(v1);
        void *inject = dlsym(handle, v1);
#ifdef DEBUG
        LOGI("inject: %p", inject);
#endif
        if (inject != NULL) {
            antied = doAntiEdXposed(env, *((jobject *) inject));
        }
        dlclose(handle);
    }
    return antied;
}
#endif

static inline void fill_jniRegisterNativeMethods(char v[]) {
    // jniRegisterNativeMethods
    v[0x0] = 'n';
    v[0x1] = 'k';
    v[0x2] = 'o';
    v[0x3] = 'U';
    v[0x4] = 'm';
    v[0x5] = 'n';
    v[0x6] = 'c';
    v[0x7] = 'x';
    v[0x8] = 'x';
    v[0x9] = 'h';
    v[0xa] = '|';
    v[0xb] = 'A';
    v[0xc] = 'q';
    v[0xd] = 'e';
    v[0xe] = '{';
    v[0xf] = 'e';
    v[0x10] = 'e';
    v[0x11] = 'L';
    v[0x12] = 'g';
    v[0x13] = 'w';
    v[0x14] = 'l';
    v[0x15] = 'j';
    v[0x16] = 'b';
    v[0x17] = 't';
    for (int i = 0; i < 0x18; ++i) {
        v[i] ^= ((i + 0x18) % 20);
    }
    v[0x18] = '\0';
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
    char prop[PROP_VALUE_MAX] = {0};

    __system_property_get("ro.build.version.sdk", prop);
    sdk = atoi(prop);

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

#ifdef SUPPORT_EPIC
    if (sdk >= 26) {
        epic = antiEpic(env);
    }
#endif

#ifdef ANTI_EDXPOSED
    if (sdk >= 23) {
        edXposed = antiEdXposed(env);
    }
#endif

    genuine = checkGenuine();
    if (genuine == CHECK_TRUE) {
        char v[0x80];
        Symbol symbol;
        fill_jniRegisterNativeMethods(v);
        if (dl_iterate_phdr_symbol(&symbol, v)) {
            genuine = CHECK_FALSE;
        }
    }

    env->DeleteLocalRef(clazz);

#ifdef DEBUG
    LOGI("JNI_OnLoad end, genuine: %d, onError: %d", genuine, throwOnError);
#endif

    if (sdk >= 24 && throwOnError && genuine == CHECK_ERROR) {
        if (sdk >= 26) {
            clearHandler(env);
        }
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}
#endif
