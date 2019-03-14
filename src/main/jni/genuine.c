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

#include "plt.h"
#include "anti-xposed.h"
#include "apk-sign-v2.h"
#include "genuine_extra.h"

#ifdef CHECK_MOUNT
#include "mount.h"
#endif

#include "epic.h"

#ifndef TAG
#define TAG "Genuine"
#endif
#ifndef LOGI
#define LOGI(...) (__genuine_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__))
#endif
#ifndef LOGW
#define LOGW(...) (__genuine_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__))
#endif
#ifndef LOGE
#define LOGE(...) (__genuine_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__))
#endif

#ifndef __genuine_log_print
__attribute__((__format__ (__printf__, 2, 0)))
static void __genuine_log_print(int prio, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    __android_log_vprint(prio, TAG, fmt, ap);
    va_end(ap);
}
#endif

enum {
    CHECK_UNKNOWN = -1,
    CHECK_TRUE,
    CHECK_FALSE,
    CHECK_FAKE,
    CHECK_ERROR,
};

#if VERSION <= CHECK_ERROR
#error "VERSION should be larger than CHECK_ERROR"
#endif

#ifdef __cplusplus
extern "C" {
#endif

static int genuine;

static int sdk;

static int uid;

static bool onError = false;

static bool xposed = false;

#ifdef __cplusplus
}
#endif

static jint version(JNIEnv *env __unused, jclass clazz __unused) {
    if (uid < 10000 || genuine == CHECK_TRUE) {
        return VERSION;
    } else {
        return genuine;
    }
}

static inline char *getGenuineClassName() {
#ifdef GET_GENUINE_CLASS_NAME
    return GET_GENUINE_CLASS_NAME();
#else
#ifndef GENUINE_CLAZZ
#define GENUINE_CLAZZ "me/piebridge/Genuine"
#endif
    return strdup(GENUINE_CLAZZ);
#endif
}

static inline char *getGenuinePackageName() {
#ifdef GET_GENUINE_PACKAGE_NAME
    return GET_GENUINE_PACKAGE_NAME();
#else
    static unsigned int m = 0;
    if (m == 0) {
        m = 20;
    } else if (m == 23) {
        m = 29;
    }
    char name[] = GENUINE_NAME;
    unsigned int length = sizeof(name) - 1;
    for (unsigned int i = 0; i < length; ++i) {
        name[i] ^= ((i + length) % m);
    }
    name[length] = '\0';
    return strdup(name);
#endif
}

static inline void fill_maps(char v[]) {
    static unsigned int m = 0;
    if (m == 0) {
        m = 13;
    } else if (m == 17) {
        m = 19;
    }
    v[0x0] = '-';
    v[0x1] = 's';
    v[0x2] = 'v';
    v[0x3] = 'j';
    v[0x4] = 'e';
    v[0x5] = '(';
    v[0x6] = '{';
    v[0x7] = 'l';
    v[0x8] = 'f';
    v[0x9] = 'm';
    v[0xa] = '#';
    v[0xb] = 'm';
    v[0xc] = '`';
    v[0xd] = 'r';
    v[0xe] = 'p';
    for (unsigned int i = 0; i < 0xf; ++i) {
        v[i] ^= ((i + 0xf) % m);
    }
    v[0xf] = '\0';
}

static inline void fill_faccessat(char v[]) {
    // faccessat
    static unsigned int m = 0;

    if (m == 0) {
        m = 7;
    } else if (m == 11) {
        m = 13;
    }

    v[0x0] = 'd';
    v[0x1] = 'b';
    v[0x2] = 'g';
    v[0x3] = 'f';
    v[0x4] = 'c';
    v[0x5] = 's';
    v[0x6] = 'r';
    v[0x7] = 'c';
    v[0x8] = 'w';
    for (unsigned int i = 0; i < 0x9; ++i) {
        v[i] ^= ((i + 0x9) % m);
    }
    v[0x9] = '\0';
}

static inline void fill_data_data_proc_self_maps(char v[]) {
    // /data/data/%s/proc_self_maps
    static unsigned int m = 0;

    if (m == 0) {
        m = 23;
    } else if (m == 29) {
        m = 31;
    }

    v[0x0] = '*';
    v[0x1] = 'b';
    v[0x2] = 'f';
    v[0x3] = '|';
    v[0x4] = 'h';
    v[0x5] = '%';
    v[0x6] = 'o';
    v[0x7] = 'm';
    v[0x8] = 'y';
    v[0x9] = 'o';
    v[0xa] = ' ';
    v[0xb] = '5';
    v[0xc] = 'b';
    v[0xd] = '=';
    v[0xe] = 'c';
    v[0xf] = 'f';
    v[0x10] = 'z';
    v[0x11] = 'u';
    v[0x12] = '_';
    v[0x13] = 'r';
    v[0x14] = 'g';
    v[0x15] = 'o';
    v[0x16] = 'b';
    v[0x17] = 'Z';
    v[0x18] = 'k';
    v[0x19] = 'f';
    v[0x1a] = 'x';
    v[0x1b] = 'z';
    for (unsigned int i = 0; i < 0x1c; ++i) {
        v[i] ^= ((i + 0x1c) % m);
    }
    v[0x1c] = '\0';
}

static inline void fill_symlink(char v[]) {
    // symlink
    static unsigned int m = 0;

    if (m == 0) {
        m = 5;
    } else if (m == 7) {
        m = 11;
    }

    v[0x0] = 'q';
    v[0x1] = 'z';
    v[0x2] = 'i';
    v[0x3] = 'l';
    v[0x4] = 'h';
    v[0x5] = 'l';
    v[0x6] = 'h';
    for (unsigned int i = 0; i < 0x7; ++i) {
        v[i] ^= ((i + 0x7) % m);
    }
    v[0x7] = '\0';
}

static inline void fill_ln_s_s_s(char v[]) {
    // ln -s %s %s
    static unsigned int m = 0;

    if (m == 0) {
        m = 7;
    } else if (m == 11) {
        m = 13;
    }

    v[0x0] = 'h';
    v[0x1] = 'k';
    v[0x2] = '&';
    v[0x3] = '-';
    v[0x4] = 'r';
    v[0x5] = '"';
    v[0x6] = '&';
    v[0x7] = 'w';
    v[0x8] = '%';
    v[0x9] = '#';
    v[0xa] = 's';
    for (unsigned int i = 0; i < 0xb; ++i) {
        v[i] ^= ((i + 0xb) % m);
    }
    v[0xb] = '\0';
}

static inline void fill_link(char link[]) {
    char format[0x1d];
    fill_data_data_proc_self_maps(format);

    char *genuinePackageName = getGenuinePackageName();
    sprintf(link, format, genuinePackageName);
#ifdef GENUINE_NAME
    free(genuinePackageName);
#endif
    unlink(link);
}

static inline bool fill_proc_self_maps(char link[]) {
    if (uid < 10000) {
        fill_maps(link);
        return true;
    }

    char target[0x10];
    fill_maps(target);
    if (sdk >= 23 && faccessat(AT_FDCWD, target, R_OK, 0) ==
                     faccessat(AT_FDCWD, target, R_OK, AT_SYMLINK_NOFOLLOW)) {
        char error[0xa];
        fill_faccessat(error);
        LOGE(error);
        return false;
    }

    char symlink[0x8];
    fill_symlink(symlink);
    Symbol symbol;
    dl_iterate_phdr_symbol(&symbol, symlink);
    int (*ln)(const char *, const char *);
    ln = (int (*)(const char *, const char *)) symbol.symbol_sym;

    if (!ln) {
        LOGE(symlink);
        return false;
    }

    fill_link(link);
    if (ln(target, link)) {
        char error[0xc];
        fill_ln_s_s_s(error);
        LOGE(error, link, target);
        return false;
    }

#ifdef DEBUG
    LOGI("maps %s", link);
#endif

    return true;
}

static inline void fill_r(char v[]) {
    static unsigned int m = 0;

    if (m == 0) {
        m = 2;
    } else if (m == 3) {
        m = 5;
    }

    v[0x0] = 's';
    for (unsigned int i = 0; i < 0x1; ++i) {
        v[i] ^= ((i + 0x1) % m);
    }
    v[0x1] = '\0';
}

static inline void rstrip(char *line) {
    char *path = line;
    if (line != NULL) {
        while (*path && *path != '\r' && *path != '\n') {
            ++path;
        }
        if (*path) {
            *path = '\0';
        }
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

static inline bool isso(const char *str) {
    const char *dot = strrchr(str, '.');
    return dot != NULL
           && *++dot == 's'
           && *++dot == 'o'
           && (*++dot == '\0' || *dot == '\r' || *dot == '\n');
}

#ifdef ANTI_ODEX
static inline size_t fill_dex2oat_cmdline(char v[]) {
    // dex2oat-cmdline
    static unsigned int m = 0;

    if (m == 0) {
        m = 13;
    } else if (m == 17) {
        m = 19;
    }

    v[0x0] = 'f';
    v[0x1] = 'f';
    v[0x2] = '|';
    v[0x3] = '7';
    v[0x4] = 'i';
    v[0x5] = 'f';
    v[0x6] = '|';
    v[0x7] = '$';
    v[0x8] = 'i';
    v[0x9] = 'f';
    v[0xa] = 'h';
    v[0xb] = 'l';
    v[0xc] = 'h';
    v[0xd] = 'l';
    v[0xe] = 'f';
    for (unsigned int i = 0; i < 0xf; ++i) {
        v[i] ^= ((i + 0xf) % m);
    }
    v[0xf] = '\0';
    return 0xf;
}

static inline size_t fill_dex_file(char v[]) {
    // --dex-file
    static unsigned int m = 0;

    if (m == 0) {
        m = 7;
    } else if (m == 11) {
        m = 13;
    }

    v[0x0] = '.';
    v[0x1] = ')';
    v[0x2] = 'a';
    v[0x3] = 'c';
    v[0x4] = 'x';
    v[0x5] = ',';
    v[0x6] = 'd';
    v[0x7] = 'j';
    v[0x8] = 'h';
    v[0x9] = '`';
    for (unsigned int i = 0; i < 0xa; ++i) {
        v[i] ^= ((i + 0xa) % m);
    }
    v[0xa] = '\0';
    return 0xa;
}

static inline int checkOdex(const char *path) {
    size_t len;
    char *cmdline;
    char buffer[0x400], find[64];

    int ret = 0;
    int fd = open(path, (unsigned) O_RDONLY | (unsigned) O_CLOEXEC);
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
                LOGE(find);
                LOGE(cmdline);
            }
            break;
        }
    }

    close(fd);

    return ret;
}
#endif

static inline bool isSameFile(char *path1, char *path2) {
    struct stat stat1, stat2;
    if (lstat(path1, &stat1)) {
        return false;
    }
    if (lstat(path2, &stat2)) {
        return false;
    }
    return stat1.st_dev == stat2.st_dev && stat1.st_ino == stat2.st_ino;
}

static inline void fill_ba88(char v[]) {
    // ba887869b4a4a6d1e915d383fad97c49
    static unsigned int m = 0;
    if (m == 0) {
        m = 13;
    } else if (m == 17) {
        m = 19;
    }
    v[0x0] = 'f';
    v[0x1] = 'o';
    v[0x2] = '[';
    v[0x3] = 'l';
    v[0x4] = 'r';
    v[0x5] = 'b';
    v[0x6] = 'z';
    v[0x7] = 'h';
    v[0x8] = '~';
    v[0x9] = 'n';
    v[0xa] = 'S';
    v[0xb] = 'p';
    v[0xc] = 'i';
    v[0xd] = 'f';
    v[0xe] = 'q';
    for (unsigned int i = 0; i < 0xf; ++i) {
        v[i] ^= ((i + 0xf) % m);
    }
    v[0xf] = '\0';
}

static inline bool isData(const char *str) {
    return str != NULL
           && *str == '/'
           && *++str == 'd'
           && *++str == 'a'
           && *++str == 't'
           && *++str == 'a'
           && *++str == '/'
           && *++str == 'a'
           && *++str == 'p'
           && *++str == 'p'
           && *++str == '/';
}

static inline bool isSame(const char *path1, const char *path2) {
    if (path1[0] == '/') {
        return strcmp(path1, path2) == 0;
    } else {
        return strcmp(path1, strrchr(path2, '/') + 1) == 0;
    }
}

enum {
    TYPE_NON,
    TYPE_APK,
    TYPE_DEX,
    TYPE_SO,
};

static inline int checkGenuine(const char *maps) {
    FILE *fp;
    char apk[NAME_MAX] = {0};
    char line[PATH_MAX];
    int check = CHECK_UNKNOWN;

    Symbol symbol;
    char d[0x10];
    fill_ba88(d);
    if (dl_iterate_phdr_symbol(&symbol, d)) {
        LOGE(d);
        check = CHECK_ERROR;
        goto clean2;
    }

#if __ANDROID_API__ >= 21 || !defined(__arm__)
    if (symbol.size == 0) {
        LOGE(d);
        check = CHECK_ERROR;
        goto clean2;
    }
#endif

    char mode[0x2];
    fill_r(mode);

    fp = fopen(maps, mode);
    if (fp == NULL) {
        fill_maps(apk);
        LOGW(apk);
        check = CHECK_ERROR;
        goto clean2;
    }

    char *genuinePackageName = getGenuinePackageName();
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
        } else if (symbol.size > 0 && isso(path)) {
            type = TYPE_SO;
            bool found = false;
            for (int i = 0; i < symbol.size; ++i) {
                if (symbol.names[i] != NULL) {
                    if (isSame(symbol.names[i], path)) {
                        free(symbol.names[i]);
                        symbol.names[i] = NULL;
                    } else {
                        found = true;
                    }
                }
            }
            if (!found) {
                symbol.size = 0;
            }
        } else {
            type = TYPE_NON;
        }
        if (strstr(path, genuinePackageName) != NULL) {
            if (type == TYPE_APK && access(path, F_OK) == 0) {
#ifdef DEBUG
                LOGI("check %s", path);
#endif
                if (apk[0] != 0) {
                    if (!strcmp(path, apk) || isSameFile(path, apk)) {
                        check = CHECK_TRUE;
                    } else {
                        LOGE(path);
                        LOGE(apk);
                        check = CHECK_FALSE;
                    }
                } else {
                    if (checkSignature(path)) {
                        LOGE(path);
                        check = CHECK_FAKE;
                    } else {
#ifdef DEBUG
                        LOGI(path);
#endif
                        check = CHECK_TRUE;
                        strcpy(apk, path);
                    }
                }
            } else if (type == TYPE_DEX) {
#ifdef ANTI_ODEX
#ifdef DEBUG
                LOGI("check %s", path);
#endif
                if (access(path, F_OK) == 0 && checkOdex(path)) {
                    LOGE(path);
                    check = JNI_FALSE;
                    break;
                }
#endif
            }
        } else if (type == TYPE_DEX && isData(path)) {
            if (xposed) {
                LOGW(path);
            } else {
                LOGE(path);
                check = CHECK_ERROR;
                goto clean;
            }
        } else if (type == TYPE_APK && isData(path)) {
#ifdef ANTI_OVERLAY
            LOGE(path);
            check = CHECK_FALSE;
            goto clean;
#endif
        }
    }

    if (check == CHECK_UNKNOWN) {
        check = CHECK_TRUE;
    }

clean:
#ifdef GENUINE_NAME
    free(genuinePackageName);
#endif
    fclose(fp);

clean2:
    for (int i = 0; i < symbol.size; ++i) {
        if (symbol.names[i] != NULL) {
            LOGE(symbol.names[i]);
            check = CHECK_ERROR;
            free(symbol.names[i]);
            symbol.names[i] = NULL;
        }
    }
    if (symbol.names != NULL) {
        free(symbol.names);
        symbol.names = NULL;
    }

    unlink(maps);
    return check;
}

#if 0
static inline void debug(JNIEnv *env, const char *prefix, jobject object) {
    jclass classObject = (*env)->FindClass(env, "java/lang/Object");
    jmethodID objectToString = (*env)->GetMethodID(env, classObject, "toString", "()Ljava/lang/String;");
    if (object == NULL) {
        LOGI(prefix, NULL);
    } else {
        jstring string = (jstring) (*env)->CallObjectMethod(env, object, objectToString);
        const char *value = (*env)->GetStringUTFChars(env, string, NULL);
        LOGI(prefix, value);
        (*env)->ReleaseStringUTFChars(env, string, value);
        (*env)->DeleteLocalRef(env, string);
    }
    (*env)->DeleteLocalRef(env, classObject);
}
#endif

#ifdef CHECK_JNI_REGISTER_NATIVE_METHODS
static inline void fill_jniRegisterNativeMethods(char v[]) {
    // jniRegisterNativeMethods
    static unsigned int m = 0;

    if (m == 0) {
        m = 23;
    } else if (m == 29) {
        m = 31;
    }

    v[0x0] = 'k';
    v[0x1] = 'l';
    v[0x2] = 'j';
    v[0x3] = 'V';
    v[0x4] = '`';
    v[0x5] = 'a';
    v[0x6] = 'n';
    v[0x7] = '{';
    v[0x8] = '}';
    v[0x9] = 'o';
    v[0xa] = 'y';
    v[0xb] = 'B';
    v[0xc] = 'l';
    v[0xd] = 'z';
    v[0xe] = 'f';
    v[0xf] = 'f';
    v[0x10] = 't';
    v[0x11] = '_';
    v[0x12] = 'v';
    v[0x13] = '`';
    v[0x14] = '}';
    v[0x15] = 'y';
    v[0x16] = 'd';
    v[0x17] = 'r';
    for (unsigned int i = 0; i < 0x18; ++i) {
        v[i] ^= ((i + 0x18) % m);
    }
    v[0x18] = '\0';
}
#endif

// FIXME: define methods in your own class
// FIXME: private static native Object invoke(Member m, int i, Object a, Object t, Object[] as) throws Throwable;
// FIXME: public static native int version();

static inline void fill_ro_build_version_sdk(char v[]) {
    // ro.build.version.sdk
    static unsigned int m = 0;

    if (m == 0) {
        m = 19;
    } else if (m == 23) {
        m = 29;
    }

    v[0x0] = 's';
    v[0x1] = 'm';
    v[0x2] = '-';
    v[0x3] = 'f';
    v[0x4] = 'p';
    v[0x5] = 'o';
    v[0x6] = 'k';
    v[0x7] = 'l';
    v[0x8] = '\'';
    v[0x9] = '|';
    v[0xa] = 'n';
    v[0xb] = '~';
    v[0xc] = '~';
    v[0xd] = 'g';
    v[0xe] = '`';
    v[0xf] = '~';
    v[0x10] = '?';
    v[0x11] = 'a';
    v[0x12] = 'd';
    v[0x13] = 'j';
    for (unsigned int i = 0; i < 0x14; ++i) {
        v[i] ^= ((i + 0x14) % m);
    }
    v[0x14] = '\0';
}

static inline void fill_version(char v[]) {
    // version
    static unsigned int m = 0;

    if (m == 0) {
        m = 5;
    } else if (m == 7) {
        m = 11;
    }

    v[0x0] = 't';
    v[0x1] = 'f';
    v[0x2] = 'v';
    v[0x3] = 's';
    v[0x4] = 'h';
    v[0x5] = 'm';
    v[0x6] = 'm';
    for (unsigned int i = 0; i < 0x7; ++i) {
        v[i] ^= ((i + 0x7) % m);
    }
    v[0x7] = '\0';
}

static inline void fill_version_signature(char v[]) {
    // ()I
    static unsigned int m = 0;

    if (m == 0) {
        m = 2;
    } else if (m == 3) {
        m = 5;
    }

    v[0x0] = ')';
    v[0x1] = ')';
    v[0x2] = 'H';
    for (unsigned int i = 0; i < 0x3; ++i) {
        v[i] ^= ((i + 0x3) % m);
    }
    v[0x3] = '\0';
}

static inline void fill_sdk_d_genuine_d(char v[]) {
    // sdk: %d, genuine: %d
    static unsigned int m = 0;

    if (m == 0) {
        m = 19;
    } else if (m == 23) {
        m = 29;
    }

    v[0x0] = 'r';
    v[0x1] = 'f';
    v[0x2] = 'h';
    v[0x3] = '>';
    v[0x4] = '%';
    v[0x5] = '#';
    v[0x6] = 'c';
    v[0x7] = '$';
    v[0x8] = ')';
    v[0x9] = 'm';
    v[0xa] = 'n';
    v[0xb] = 'b';
    v[0xc] = 'x';
    v[0xd] = 'g';
    v[0xe] = 'a';
    v[0xf] = 'u';
    v[0x10] = '+';
    v[0x11] = '2';
    v[0x12] = '%';
    v[0x13] = 'e';
    for (unsigned int i = 0; i < 0x14; ++i) {
        v[i] ^= ((i + 0x14) % m);
    }
    v[0x14] = '\0';
}

static inline void fill_add_sigcont(char v[]) {
    // add sigcont handler
    static int m = 0;

    if (m == 0) {
        m = 17;
    } else if (m == 19) {
        m = 23;
    }

    v[0x0] = 'c';
    v[0x1] = 'g';
    v[0x2] = '`';
    v[0x3] = '%';
    v[0x4] = 'u';
    v[0x5] = 'n';
    v[0x6] = 'o';
    v[0x7] = 'j';
    v[0x8] = 'e';
    v[0x9] = 'e';
    v[0xa] = 'x';
    v[0xb] = '-';
    v[0xc] = 'f';
    v[0xd] = 'n';
    v[0xe] = '~';
    v[0xf] = 'd';
    v[0x10] = 'm';
    v[0x11] = 'g';
    v[0x12] = 'q';
    for (unsigned int i = 0; i < 0x13; ++i) {
        v[i] ^= ((i + 0x13) % m);
    }
    v[0x13] = '\0';
}

static inline void fill_received_sigcont(char v[]) {
    // received sigcont
    static int m = 0;

    if (m == 0) {
        m = 13;
    } else if (m == 17) {
        m = 19;
    }

    v[0x0] = 'q';
    v[0x1] = 'a';
    v[0x2] = 'f';
    v[0x3] = 'c';
    v[0x4] = 'n';
    v[0x5] = '~';
    v[0x6] = 'l';
    v[0x7] = 'n';
    v[0x8] = '+';
    v[0x9] = '\x7f';
    v[0xa] = 'i';
    v[0xb] = 'f';
    v[0xc] = 'a';
    v[0xd] = 'l';
    v[0xe] = 'j';
    v[0xf] = 'q';
    for (unsigned int i = 0; i < 0x10; ++i) {
        v[i] ^= ((i + 0x10) % m);
    }
    v[0x10] = '\0';
}

static void handler(int sig __unused) {
    char v[0x11];
    fill_received_sigcont(v);
    LOGI(v);
}

jint JNI_OnLoad(JavaVM *jvm, void *v __unused) {
    JNIEnv *env;
    jclass clazz;
    char v1[0x19];
    char prop[PROP_VALUE_MAX] = {0};

    signal(SIGCONT, handler);
    fill_add_sigcont(v1);
    LOGI(v1); // 0x14

    fill_ro_build_version_sdk(v1); // 0x15
    __system_property_get(v1, prop);
    sdk = (int) strtol(prop, NULL, 10);;

    uid = getuid();

#ifdef DEBUG
    LOGI("JNI_OnLoad start");
#endif

    if ((*jvm)->GetEnv(jvm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    char *genuineClassName = getGenuineClassName();
    if ((clazz = (*env)->FindClass(env, genuineClassName)) == NULL) {
        return JNI_ERR;
    }
#ifdef GENUINE_CLAZZ
    free(genuineClassName);
#endif

    JNINativeMethod methods[1];
    fill_version(v1); // v1: 0x8
    methods[0].name = strdup(v1);

    fill_version_signature(v1); // v1: 0x4
    methods[0].signature = strdup(v1);
    methods[0].fnPtr = version;

    if ((*env)->RegisterNatives(env, clazz, methods, 1) < 0) {
        return JNI_ERR;
    }

#ifdef DEBUG
    LOGI("JNI_OnLoad_Extra start");
#endif
    if (JNI_OnLoad_Extra(env, clazz, sdk, &onError) < 0) {
        return JNI_ERR;
    }

    if (uid < 10000) {
        (*env)->DeleteLocalRef(env, clazz);
        return JNI_VERSION_1_6;
    }

    char maps[NAME_MAX];
    bool safe = true;
    if (!fill_proc_self_maps(maps)) {
        fill_maps(maps);
        safe = false;
    }

    if (sdk >= 21) {
#ifdef DEBUG
        LOGI("antiXposed start");
#endif
        antiXposed(env, clazz, maps, sdk, &xposed);

#ifdef DEBUG
        LOGI("antiEdXposed start");
#endif
        antiEdXposed(env);

#ifdef SUPPORT_EPIC
        antiEpic(env, sdk);
#endif
    }

#ifdef CHECK_MOUNT
    checkMount(maps);
#endif

#ifdef DEBUG
    LOGI("checkGenuine start");
#endif
    genuine = checkGenuine(maps);
    if (genuine == CHECK_TRUE && !safe) {
        genuine = CHECK_FALSE;
    }

#ifdef CHECK_JNI_REGISTER_NATIVE_METHODS
    if (genuine == CHECK_TRUE) {
        Symbol symbol;
        fill_jniRegisterNativeMethods(v1); // 0x19
        if (dl_iterate_phdr_symbol(&symbol, v1)) {
            LOGW(v1);
            genuine = CHECK_FALSE;
        }
    }
#endif

    (*env)->DeleteLocalRef(env, clazz);

#ifdef DEBUG
    LOGI("JNI_OnLoad end, genuine: %d, onError: %d", genuine, onError);
#endif

    if (onError && genuine == CHECK_ERROR) {
        clearHandler(env);
        return JNI_ERR;
    }

    if (genuine != CHECK_TRUE) {
        fill_sdk_d_genuine_d(v1); // 0x15
        LOGI(v1, sdk, genuine);
    }

    return JNI_VERSION_1_6;
}
