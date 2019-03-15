//
// Created by Thom on 2019/2/16.
//

#include <stdbool.h>
#include <string.h>
#include <jni.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include "genuine.h"
#include "plt.h"
#include "libraries-mock.h"

#ifndef TAG
#define TAG "Genuine"
#endif

#ifndef LOGI
#define LOGI(...) (__android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__))
#endif
#ifndef LOGW
#define LOGW(...) (__android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__))
#endif

#if (defined(CHECK_XPOSED_EPIC) || defined(CHECK_SO_LIBRARY)) && defined(GENUINE_NO_STL)

typedef struct l {
    size_t cap __unused;
    size_t size __unused;
    char *data;
} l;

typedef struct s {
    unsigned char size;
    char data[sizeof(l) - sizeof(unsigned char)];
} s;

typedef struct string {
    union {
        l l;
        s s;
    };
} string;

#include <endian.h>

#if BYTE_ORDER == LITTLE_ENDIAN
#define SHORT_MASK 0x01U
#elif BYTE_ORDER == BIG_ENDIAN
#define SHORT_MASK 0x80U
#else
#error "invalid byte order"
#endif

static const char *get_data(const string *string) {
    unsigned char size = string->s.size;
    if ((size & SHORT_MASK)) {
        return string->l.data;
    } else {
        return string->s.data;
    }
}

struct tree;

typedef struct end {
    struct tree *left;
} end;

typedef struct tree {
    struct tree *left;
    struct tree *right;
    struct tree *parent;
    bool black __unused;
    string first;
    uintptr_t *second;
} tree;

typedef struct map {
    tree *begin;
    end *end;
    size_t size;
} map;

typedef struct SharedLibrary {
    const string path;

    // The void* returned by dlopen(3).
    void *const handle __unused;

    // True if a native bridge is required.
    bool needs_native_bridge __unused;

    // The ClassLoader this library is associated with, a weak global JNI reference that is
    // created/deleted with the scope of the library.
    const jweak class_loader;
} SharedLibrary;

#ifdef DEBUG

static inline void debug(JNIEnv *env, const char *prefix, jobject object) {
    if (object == NULL) {
        LOGI(prefix, NULL);
    } else {
        jclass objectClass = (*env)->GetObjectClass(env, object);
        jmethodID toString = (*env)->GetMethodID(env, objectClass, "toString",
                                                 "()Ljava/lang/String;");
        jstring string = (jstring) (*env)->CallObjectMethod(env, object, toString);
        const char *value = (*env)->GetStringUTFChars(env, string, NULL);
        LOGI(prefix, value);
        (*env)->ReleaseStringUTFChars(env, string, value);
        (*env)->DeleteLocalRef(env, string);
        (*env)->DeleteLocalRef(env, objectClass);
    }
}

#endif

static inline void fill_DumpForSigQuit(char v[]) {
    // _ZN3art9JavaVMExt14DumpForSigQuitERNSt3__113basic_ostreamIcNS1_11char_traitsIcEEEE
    static int m = 0;

    if (m == 0) {
        m = 79;
    } else if (m == 83) {
        m = 89;
    }

    v[0x0] = '\\';
    v[0x1] = '^';
    v[0x2] = 'K';
    v[0x3] = '5';
    v[0x4] = 'f';
    v[0x5] = 'z';
    v[0x6] = '}';
    v[0x7] = '3';
    v[0x8] = 'A';
    v[0x9] = 'm';
    v[0xa] = '{';
    v[0xb] = 'o';
    v[0xc] = 'Y';
    v[0xd] = ']';
    v[0xe] = 'T';
    v[0xf] = 'j';
    v[0x10] = 'g';
    v[0x11] = '%';
    v[0x12] = '!';
    v[0x13] = 'R';
    v[0x14] = 'b';
    v[0x15] = 'u';
    v[0x16] = 'i';
    v[0x17] = '\\';
    v[0x18] = 't';
    v[0x19] = 'n';
    v[0x1a] = 'N';
    v[0x1b] = 'w';
    v[0x1c] = 'x';
    v[0x1d] = 'q';
    v[0x1e] = 'T';
    v[0x1f] = 'K';
    v[0x20] = 'W';
    v[0x21] = 'a';
    v[0x22] = 'w';
    v[0x23] = 'h';
    v[0x24] = 't';
    v[0x25] = '\\';
    v[0x26] = '\x1a';
    v[0x27] = 'u';
    v[0x28] = 't';
    v[0x29] = '\x1d';
    v[0x2a] = '\x1c';
    v[0x2b] = '\x1d';
    v[0x2c] = 'M';
    v[0x2d] = 'Q';
    v[0x2e] = 'B';
    v[0x2f] = '[';
    v[0x30] = 'P';
    v[0x31] = 'k';
    v[0x32] = 'Z';
    v[0x33] = 'E';
    v[0x34] = 'C';
    v[0x35] = 'J';
    v[0x36] = '\\';
    v[0x37] = '[';
    v[0x38] = 'V';
    v[0x39] = 'u';
    v[0x3a] = '^';
    v[0x3b] = 'p';
    v[0x3c] = 'l';
    v[0x3d] = 'q';
    v[0x3e] = '\x1e';
    v[0x3f] = 's';
    v[0x40] = 'r';
    v[0x41] = '\'';
    v[0x42] = '-';
    v[0x43] = '\'';
    v[0x44] = '5';
    v[0x45] = '\x17';
    v[0x46] = '=';
    v[0x47] = '8';
    v[0x48] = '*';
    v[0x49] = '%';
    v[0x4a] = '9';
    v[0x4b] = '=';
    v[0x4c] = 'I';
    v[0x4d] = 'b';
    v[0x4e] = 'G';
    v[0x4f] = 'F';
    v[0x50] = 'A';
    v[0x51] = '@';
    for (unsigned int i = 0; i < 0x52; ++i) {
        v[i] ^= ((i + 0x52) % m);
    }
    v[0x52] = '\0';
}

static inline void fill_dev_random(char v[]) {
    // /dev/random
    static int m = 0;

    if (m == 0) {
        m = 7;
    } else if (m == 11) {
        m = 13;
    }

    v[0x0] = '+';
    v[0x1] = 'a';
    v[0x2] = 'c';
    v[0x3] = 'v';
    v[0x4] = '.';
    v[0x5] = 'p';
    v[0x6] = 'b';
    v[0x7] = 'j';
    v[0x8] = 'a';
    v[0x9] = 'i';
    v[0xa] = 'm';
    for (unsigned int i = 0; i < 0xb; ++i) {
        v[i] ^= ((i + 0xb) % m);
    }
    v[0xb] = '\0';
}

static void inline fill_NewLocalRef(char v[]) {
    // _ZN3art9JNIEnvExt11NewLocalRefEPNS_6mirror6ObjectE
    static int m = 0;

    if (m == 0) {
        m = 47;
    } else if (m == 53) {
        m = 59;
    }

    v[0x0] = '\\';
    v[0x1] = '^';
    v[0x2] = 'K';
    v[0x3] = '5';
    v[0x4] = 'f';
    v[0x5] = 'z';
    v[0x6] = '}';
    v[0x7] = '3';
    v[0x8] = 'A';
    v[0x9] = 'B';
    v[0xa] = 'D';
    v[0xb] = 'K';
    v[0xc] = 'a';
    v[0xd] = 'f';
    v[0xe] = 'T';
    v[0xf] = 'j';
    v[0x10] = 'g';
    v[0x11] = '%';
    v[0x12] = '$';
    v[0x13] = 'X';
    v[0x14] = 'r';
    v[0x15] = 'o';
    v[0x16] = 'U';
    v[0x17] = 'u';
    v[0x18] = 'x';
    v[0x19] = '}';
    v[0x1a] = 'q';
    v[0x1b] = 'L';
    v[0x1c] = 'z';
    v[0x1d] = 'F';
    v[0x1e] = 'd';
    v[0x1f] = 'r';
    v[0x20] = 'm';
    v[0x21] = 'w';
    v[0x22] = 'z';
    v[0x23] = '\x10';
    v[0x24] = 'J';
    v[0x25] = 'A';
    v[0x26] = '[';
    v[0x27] = 'X';
    v[0x28] = 'D';
    v[0x29] = '^';
    v[0x2a] = '\x1b';
    v[0x2b] = 'a';
    v[0x2c] = 'b';
    v[0x2d] = 'k';
    v[0x2e] = 'g';
    v[0x2f] = '`';
    v[0x30] = 'p';
    v[0x31] = '@';
    for (unsigned int i = 0; i < 0x32; ++i) {
        v[i] ^= ((i + 0x32) % m);
    }
    v[0x32] = '\0';
}

static tree *tree_min(tree *t) {
    while (t->left) {
        t = t->left;
    }
    return t;
}

static bool tree_is_left_child(tree *t) {
    return t->parent != NULL && t == t->parent->left;
}

static tree *tree_next(tree *t) {
    if (t->right) {
        return tree_min(t->right);
    }
    while (!tree_is_left_child(t)) {
        t = t->parent;
    }
    return t->parent;
}

static inline bool issystem(const char *str) {
    return str != NULL
           && *str == '/'
           && *++str == 's'
           && *++str == 'y'
           && *++str == 's'
           && *++str == 't'
           && *++str == 'e'
           && *++str == 'm'
           && *++str == '/';
}

static inline jobject checkClassLoader(JNIEnv *env, int sdk, SharedLibrary *sharedLibrary) {
    jobject classLoader = sharedLibrary->class_loader;
#ifdef DEBUG
    LOGI("path: %s, %p", get_data(&(sharedLibrary->path)), classLoader);
#endif
    if (classLoader == NULL) {
        return NULL;
    }
    if (sdk < 23) {
        char v[0x33];
        Symbol symbol;
        fill_NewLocalRef(v);
        dl_iterate_phdr_symbol(&symbol, v);
        classLoader = ((jobject (*)(JNIEnv *, void *)) (symbol.symbol_sym))(env, classLoader);
    }
#ifdef DEBUG
    debug(env, "%s", classLoader);
#endif
    return classLoader;
}

jobject findClassLoader(JNIEnv *env, const char *name, int sdk) {
    if (sdk < 21) {
        return NULL;
    }

    Symbol symbol;
    char v[0x53];
    fill_DumpForSigQuit(v);
    dl_iterate_phdr_symbol(&symbol, v);

    JavaVM *jvm;
    (*env)->GetJavaVM(env, &jvm);

    if (symbol.symbol_sym == NULL) {
        return NULL;
    }

#ifdef DUMP_JVM
    char *dump = dump_jvm(jvm, &symbol);
#ifdef DEBUG
    LOGI("DumpForSigQuit: %s", dump);
#endif
    bool safe = dump != NULL && strstr(dump, name) == NULL;
    free(dump);
    if (safe) {
        return NULL;
    }
#endif

    fill_dev_random(v);
    int random = open(v, (unsigned) O_WRONLY | (unsigned) O_CLOEXEC);

    map *found = NULL;
    map **ptr = (map **) jvm;
    // skip functions, runtime
    for (int i = 0; i < 42; ++i, ++ptr) {
        map *map = *ptr;
        if (map != NULL) {
            // begin, pair1, pair3
            uintptr_t *pair1 = (uintptr_t *) map + 1;
            uintptr_t *pair3 = (uintptr_t *) map + 2;
            if (write(random, map, sizeof(uintptr_t *)) >= 0
                && write(random, pair1, sizeof(uintptr_t *)) >= 0
                && write(random, pair3, sizeof(uintptr_t *)) >= 0
                && map->size > 1 && map->size < symbol.total) {
#ifdef DEBUG
                LOGI("found libraries_ on jvm+0x%x, jvm: %p, libraries_: %p -> %p",
                     i * sizeof(uintptr_t), jvm, ptr, map);
#endif
                found = map;
                break;
            }
        }
    }

    close(random);

    if (found != NULL) {
        tree *t = found->begin;
        void *end = &(found->end);
        for (size_t i = 0; i < found->size; ++i) {
            const char *data = get_data(&(t->first));
#ifdef DEBUG
            LOGI("tree: %p, path: %s, loader: %p", t, data, t->second);
#endif
            if (data != NULL) {
                if (strstr(data, name) != NULL && t->second != NULL) {
                    jobject classLoader = checkClassLoader(env, sdk, (SharedLibrary *) t->second);
                    if (classLoader != NULL) {
                        return classLoader;
                    }
                } else if (issystem(data)) {
                    break;
                }
            }
            tree *n = tree_next(t);
            if (!n || (void *) n == end) {
                break;
            }
            t = n;
        }
    }

    return NULL;
}

#endif