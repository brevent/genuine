#include "epic.h"
#include "libraries.h"

#ifdef CHECK_XPOSED_EPIC

#ifdef DEBUG

static inline void debug(JNIEnv *env, const char *prefix, jobject object) {
    jclass classObject = (*env)->FindClass(env, "java/lang/Object");
    jmethodID objectToString = (*env)->GetMethodID(env, classObject, "toString",
                                                   "()Ljava/lang/String;");
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

#else
#define debug(x, y, z) do {} while(0);
#endif

static inline void fill_hookedMethodCallbacks(char v[]) {
    // hookedMethodCallbacks
    static unsigned int m = 0;

    if (m == 0) {
        m = 19;
    } else if (m == 23) {
        m = 29;
    }

    v[0x0] = 'j';
    v[0x1] = 'l';
    v[0x2] = 'k';
    v[0x3] = 'n';
    v[0x4] = 'c';
    v[0x5] = 'c';
    v[0x6] = 'E';
    v[0x7] = 'l';
    v[0x8] = '~';
    v[0x9] = 'c';
    v[0xa] = 'c';
    v[0xb] = 'i';
    v[0xc] = 'M';
    v[0xd] = 'n';
    v[0xe] = '|';
    v[0xf] = '}';
    v[0x10] = 'p';
    v[0x11] = 'a';
    v[0x12] = 'b';
    v[0x13] = 'i';
    v[0x14] = 'p';
    for (unsigned int i = 0; i < 0x15; ++i) {
        v[i] ^= ((i + 0x15) % m);
    }
    v[0x15] = '\0';
}


static inline void fill_hookedMethodCallbacks_signature(char v[]) {
    // Ljava/util/Map;
    static unsigned int m = 0;

    if (m == 0) {
        m = 13;
    } else if (m == 17) {
        m = 19;
    }

    v[0x0] = 'N';
    v[0x1] = 'i';
    v[0x2] = 'e';
    v[0x3] = 's';
    v[0x4] = 'g';
    v[0x5] = '(';
    v[0x6] = '}';
    v[0x7] = '}';
    v[0x8] = 'c';
    v[0x9] = 'g';
    v[0xa] = '#';
    v[0xb] = 'M';
    v[0xc] = '`';
    v[0xd] = 'r';
    v[0xe] = '8';
    for (unsigned int i = 0; i < 0xf; ++i) {
        v[i] ^= ((i + 0xf) % m);
    }
    v[0xf] = '\0';
}

static inline void fill_java_util_Map(char v[]) {
    // java/util/Map
    static unsigned int m = 0;

    if (m == 0) {
        m = 11;
    } else if (m == 13) {
        m = 17;
    }

    v[0x0] = 'h';
    v[0x1] = 'b';
    v[0x2] = 'r';
    v[0x3] = 'd';
    v[0x4] = ')';
    v[0x5] = 'r';
    v[0x6] = '|';
    v[0x7] = '`';
    v[0x8] = 'f';
    v[0x9] = '/';
    v[0xa] = 'L';
    v[0xb] = 'c';
    v[0xc] = 's';
    for (unsigned int i = 0; i < 0xd; ++i) {
        v[i] ^= ((i + 0xd) % m);
    }
    v[0xd] = '\0';
}

static inline void fill_put(char v[]) {
    // put
    static unsigned int m = 0;

    if (m == 0) {
        m = 2;
    } else if (m == 3) {
        m = 5;
    }

    v[0x0] = 'q';
    v[0x1] = 'u';
    v[0x2] = 'u';
    for (unsigned int i = 0; i < 0x3; ++i) {
        v[i] ^= ((i + 0x3) % m);
    }
    v[0x3] = '\0';
}

static inline void fill_put_signature(char v[]) {
    // (Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;
    static unsigned int m = 0;

    if (m == 0) {
        m = 53;
    } else if (m == 59) {
        m = 61;
    }

    v[0x0] = '+';
    v[0x1] = 'H';
    v[0x2] = 'o';
    v[0x3] = 'g';
    v[0x4] = 'q';
    v[0x5] = 'i';
    v[0x6] = '&';
    v[0x7] = 'f';
    v[0x8] = 'j';
    v[0x9] = 'b';
    v[0xa] = 'j';
    v[0xb] = '!';
    v[0xc] = '@';
    v[0xd] = 'r';
    v[0xe] = '{';
    v[0xf] = 'w';
    v[0x10] = 'p';
    v[0x11] = '`';
    v[0x12] = '.';
    v[0x13] = 'Z';
    v[0x14] = '}';
    v[0x15] = 'y';
    v[0x16] = 'o';
    v[0x17] = '{';
    v[0x18] = '4';
    v[0x19] = 'p';
    v[0x1a] = '|';
    v[0x1b] = 'p';
    v[0x1c] = 'x';
    v[0x1d] = '\x0f';
    v[0x1e] = 'n';
    v[0x1f] = '@';
    v[0x20] = 'I';
    v[0x21] = 'A';
    v[0x22] = 'F';
    v[0x23] = 'R';
    v[0x24] = '\x1c';
    v[0x25] = '\x01';
    v[0x26] = 'e';
    v[0x27] = '@';
    v[0x28] = 'J';
    v[0x29] = 'Z';
    v[0x2a] = 'L';
    v[0x2b] = '\x01';
    v[0x2c] = 'C';
    v[0x2d] = 'Q';
    v[0x2e] = '_';
    v[0x2f] = 'U';
    v[0x30] = '\x1c';
    v[0x31] = '{';
    v[0x32] = 'b';
    v[0x33] = 'k';
    v[0x34] = 'g';
    v[0x35] = '`';
    v[0x36] = 'p';
    v[0x37] = '>';
    for (unsigned int i = 0; i < 0x38; ++i) {
        v[i] ^= ((i + 0x38) % m);
    }
    v[0x38] = '\0';
}

static inline void fill_entrySet(char v[]) {
    // entrySet
    static unsigned int m = 0;

    if (m == 0) {
        m = 7;
    } else if (m == 11) {
        m = 13;
    }

    v[0x0] = 'd';
    v[0x1] = 'l';
    v[0x2] = 'w';
    v[0x3] = 'v';
    v[0x4] = '|';
    v[0x5] = 'U';
    v[0x6] = 'e';
    v[0x7] = 'u';
    for (unsigned int i = 0; i < 0x8; ++i) {
        v[i] ^= ((i + 0x8) % m);
    }
    v[0x8] = '\0';
}

static inline void fill_entrySet_signature(char v[]) {
    // ()Ljava/util/Set;
    static unsigned int m = 0;

    if (m == 0) {
        m = 13;
    } else if (m == 17) {
        m = 19;
    }

    v[0x0] = ',';
    v[0x1] = ',';
    v[0x2] = 'J';
    v[0x3] = 'm';
    v[0x4] = 'i';
    v[0x5] = '\x7f';
    v[0x6] = 'k';
    v[0x7] = '$';
    v[0x8] = 'y';
    v[0x9] = 't';
    v[0xa] = 'h';
    v[0xb] = 'n';
    v[0xc] = ',';
    v[0xd] = 'W';
    v[0xe] = '`';
    v[0xf] = 'r';
    v[0x10] = '<';
    for (unsigned int i = 0; i < 0x11; ++i) {
        v[i] ^= ((i + 0x11) % m);
    }
    v[0x11] = '\0';
}

static inline void fill_java_util_Set(char v[]) {
    // java/util/Set
    static unsigned int m = 0;

    if (m == 0) {
        m = 11;
    } else if (m == 13) {
        m = 17;
    }

    v[0x0] = 'h';
    v[0x1] = 'b';
    v[0x2] = 'r';
    v[0x3] = 'd';
    v[0x4] = ')';
    v[0x5] = 'r';
    v[0x6] = '|';
    v[0x7] = '`';
    v[0x8] = 'f';
    v[0x9] = '/';
    v[0xa] = 'R';
    v[0xb] = 'g';
    v[0xc] = 'w';
    for (unsigned int i = 0; i < 0xd; ++i) {
        v[i] ^= ((i + 0xd) % m);
    }
    v[0xd] = '\0';
}

static inline void fill_iterator(char v[]) {
    // iterator
    static unsigned int m = 0;

    if (m == 0) {
        m = 7;
    } else if (m == 11) {
        m = 13;
    }

    v[0x0] = 'h';
    v[0x1] = 'v';
    v[0x2] = 'f';
    v[0x3] = 'v';
    v[0x4] = 'd';
    v[0x5] = 'r';
    v[0x6] = 'o';
    v[0x7] = 's';
    for (unsigned int i = 0; i < 0x8; ++i) {
        v[i] ^= ((i + 0x8) % m);
    }
    v[0x8] = '\0';
}

static inline void fill_iterator_signature(char v[]) {
    // ()Ljava/util/Iterator;
    static unsigned int m = 0;

    if (m == 0) {
        m = 19;
    } else if (m == 23) {
        m = 29;
    }

    v[0x0] = '+';
    v[0x1] = '-';
    v[0x2] = 'I';
    v[0x3] = 'l';
    v[0x4] = 'f';
    v[0x5] = '~';
    v[0x6] = 'h';
    v[0x7] = '%';
    v[0x8] = '~';
    v[0x9] = 'x';
    v[0xa] = 'd';
    v[0xb] = 'b';
    v[0xc] = ' ';
    v[0xd] = 'Y';
    v[0xe] = 'e';
    v[0xf] = 'w';
    v[0x10] = 'r';
    v[0x11] = '`';
    v[0x12] = 'v';
    v[0x13] = 'l';
    v[0x14] = 'v';
    v[0x15] = '>';
    for (unsigned int i = 0; i < 0x16; ++i) {
        v[i] ^= ((i + 0x16) % m);
    }
    v[0x16] = '\0';
}

static inline void fill_java_util_Iterator(char v[]) {
    // java/util/Iterator
    static unsigned int m = 0;

    if (m == 0) {
        m = 17;
    } else if (m == 19) {
        m = 23;
    }

    v[0x0] = 'k';
    v[0x1] = 'c';
    v[0x2] = 'u';
    v[0x3] = 'e';
    v[0x4] = '*';
    v[0x5] = 's';
    v[0x6] = 's';
    v[0x7] = 'a';
    v[0x8] = 'e';
    v[0x9] = '%';
    v[0xa] = 'B';
    v[0xb] = 'x';
    v[0xc] = 'h';
    v[0xd] = '|';
    v[0xe] = 'n';
    v[0xf] = 'd';
    v[0x10] = 'o';
    v[0x11] = 's';
    for (unsigned int i = 0; i < 0x12; ++i) {
        v[i] ^= ((i + 0x12) % m);
    }
    v[0x12] = '\0';
}

static inline void fill_hasNext(char v[]) {
    // hasNext
    static unsigned int m = 0;

    if (m == 0) {
        m = 5;
    } else if (m == 7) {
        m = 11;
    }

    v[0x0] = 'j';
    v[0x1] = 'b';
    v[0x2] = 'w';
    v[0x3] = 'N';
    v[0x4] = 'd';
    v[0x5] = 'z';
    v[0x6] = 'w';
    for (unsigned int i = 0; i < 0x7; ++i) {
        v[i] ^= ((i + 0x7) % m);
    }
    v[0x7] = '\0';
}

static inline void fill_hasNext_signature(char v[]) {
    // ()Z
    static unsigned int m = 0;

    if (m == 0) {
        m = 2;
    } else if (m == 3) {
        m = 5;
    }

    v[0x0] = ')';
    v[0x1] = ')';
    v[0x2] = '[';
    for (unsigned int i = 0; i < 0x3; ++i) {
        v[i] ^= ((i + 0x3) % m);
    }
    v[0x3] = '\0';
}

static inline void fill_next(char v[]) {
    // next
    static unsigned int m = 0;

    if (m == 0) {
        m = 3;
    } else if (m == 5) {
        m = 7;
    }

    v[0x0] = 'o';
    v[0x1] = 'g';
    v[0x2] = 'x';
    v[0x3] = 'u';
    for (unsigned int i = 0; i < 0x4; ++i) {
        v[i] ^= ((i + 0x4) % m);
    }
    v[0x4] = '\0';
}

static inline void fill_object_signature(char v[]) {
    // ()Ljava/lang/Object;
    static unsigned int m = 0;

    if (m == 0) {
        m = 19;
    } else if (m == 23) {
        m = 29;
    }

    v[0x0] = ')';
    v[0x1] = '+';
    v[0x2] = 'O';
    v[0x3] = 'n';
    v[0x4] = 'd';
    v[0x5] = 'p';
    v[0x6] = 'f';
    v[0x7] = '\'';
    v[0x8] = 'e';
    v[0x9] = 'k';
    v[0xa] = 'e';
    v[0xb] = 'k';
    v[0xc] = '"';
    v[0xd] = 'A';
    v[0xe] = 'm';
    v[0xf] = 'z';
    v[0x10] = 't';
    v[0x11] = 'q';
    v[0x12] = 't';
    v[0x13] = ':';
    for (unsigned int i = 0; i < 0x14; ++i) {
        v[i] ^= ((i + 0x14) % m);
    }
    v[0x14] = '\0';
}

static inline void fill_getKey(char v[]) {
    // getKey
    static unsigned int m = 0;

    if (m == 0) {
        m = 5;
    } else if (m == 7) {
        m = 11;
    }

    v[0x0] = 'f';
    v[0x1] = 'g';
    v[0x2] = 'w';
    v[0x3] = 'O';
    v[0x4] = 'e';
    v[0x5] = 'x';
    for (unsigned int i = 0; i < 0x6; ++i) {
        v[i] ^= ((i + 0x6) % m);
    }
    v[0x6] = '\0';
}

static inline void fill_getValue(char v[]) {
    // getValue
    static unsigned int m = 0;

    if (m == 0) {
        m = 7;
    } else if (m == 11) {
        m = 13;
    }

    v[0x0] = 'f';
    v[0x1] = 'g';
    v[0x2] = 'w';
    v[0x3] = 'R';
    v[0x4] = 'd';
    v[0x5] = 'j';
    v[0x6] = 'u';
    v[0x7] = 'd';
    for (unsigned int i = 0; i < 0x8; ++i) {
        v[i] ^= ((i + 0x8) % m);
    }
    v[0x8] = '\0';
}

static inline void fill_init(char v[]) {
    // <init>
    static unsigned int m = 0;

    if (m == 0) {
        m = 5;
    } else if (m == 7) {
        m = 11;
    }

    v[0x0] = '=';
    v[0x1] = 'k';
    v[0x2] = 'm';
    v[0x3] = 'm';
    v[0x4] = 't';
    v[0x5] = '?';
    for (unsigned int i = 0; i < 0x6; ++i) {
        v[i] ^= ((i + 0x6) % m);
    }
    v[0x6] = '\0';
}

static inline void fill_void_signature(char v[]) {
    // ()V
    static unsigned int m = 0;

    if (m == 0) {
        m = 2;
    } else if (m == 3) {
        m = 5;
    }

    v[0x0] = ')';
    v[0x1] = ')';
    v[0x2] = 'W';
    for (unsigned int i = 0; i < 0x3; ++i) {
        v[i] ^= ((i + 0x3) % m);
    }
    v[0x3] = '\0';
}

static inline bool doAntiEpic(JNIEnv *env, jclass classDexposedBridge) {
    char v1[0x20], v2[0x40];
    bool antied = false;

    fill_hookedMethodCallbacks(v1); // 0x16
    fill_hookedMethodCallbacks_signature(v2); // 0x10
    jfieldID field = (*env)->GetStaticFieldID(env, classDexposedBridge, v1, v2);
    if (field == NULL) {
        (*env)->ExceptionClear(env);
        return false;
    }
    jobject map = (*env)->GetStaticObjectField(env, classDexposedBridge, field);
    debug(env, "hooks: %s", map);

    fill_java_util_Map(v1); // 0xe
    jclass classMap = (*env)->FindClass(env, v1);
    if (map == NULL || !(*env)->IsInstanceOf(env, map, classMap)) {
        return false;
    }
    fill_put(v1); // 0x4
    fill_put_signature(v2); // 0x39
    jmethodID put = (*env)->GetMethodID(env, classMap, v1, v2);
    fill_entrySet(v1); // 0x9
    fill_entrySet_signature(v2); // 0x12
    jmethodID method = (*env)->GetMethodID(env, classMap, v1, v2);
    jobject entrySet = (*env)->CallObjectMethod(env, map, method);

    fill_java_util_Set(v1); // 0xe
    jclass classSet = (*env)->FindClass(env, v1);
    fill_iterator(v1); // 0x9
    fill_iterator_signature(v2); // 0x17
    method = (*env)->GetMethodID(env, classSet, v1, v2);
    jobject iterator = (*env)->CallObjectMethod(env, entrySet, method);

    fill_java_util_Iterator(v1); // 0x13
    jclass classIterator = (*env)->FindClass(env, v1);
    fill_hasNext(v1); // 0x8
    fill_hasNext_signature(v2); // 0x4
    jmethodID hasNext = (*env)->GetMethodID(env, classIterator, v1, v2);
    fill_next(v1); // 0x5
    fill_object_signature(v2); // 0x15
    jmethodID next = (*env)->GetMethodID(env, classIterator, v1, v2);

    jobject emptyHooks = NULL;
    jmethodID getKey = NULL;
    jmethodID getValue = NULL;
    while ((*env)->CallBooleanMethod(env, iterator, hasNext)) {
        jobject entry = (*env)->CallObjectMethod(env, iterator, next);
        jclass entryClass = (*env)->GetObjectClass(env, entry);
        if (getKey == NULL) {
            fill_getKey(v1); // 0x7
            fill_object_signature(v2); // 0x15
            getKey = (*env)->GetMethodID(env, entryClass, v1, v2);
        }
        if (getValue == NULL) {
            fill_getValue(v1); // 0x9
            fill_object_signature(v2); // 0x15
            getValue = (*env)->GetMethodID(env, entryClass, v1, v2);
        }

        jobject member = (*env)->CallObjectMethod(env, entry, getKey);
        debug(env, "hook member: %s", member);
        if (member == NULL) {
            continue;
        }
        jobject hooks = (*env)->CallObjectMethod(env, entry, getValue);
        debug(env, "hooks value: %s", hooks);
        if (hooks == NULL) {
            continue;
        }

        if (emptyHooks == NULL) {
            jclass hookClass = (*env)->GetObjectClass(env, hooks);
            debug(env, "hooks value class: %s", hookClass);
            fill_init(v1); // 0x7
            fill_void_signature(v2); // 0x4
            jmethodID init = (*env)->GetMethodID(env, hookClass, v1, v2);
            if (init == NULL) {
                (*env)->ExceptionClear(env);
                (*env)->DeleteLocalRef(env, hookClass);
                (*env)->DeleteLocalRef(env, entry);
                (*env)->DeleteLocalRef(env, entryClass);
                (*env)->DeleteLocalRef(env, hooks);
                (*env)->DeleteLocalRef(env, member);
                break;
            }
            emptyHooks = (*env)->NewObject(env, hookClass, init);
            (*env)->DeleteLocalRef(env, hookClass);
        }
        (*env)->CallObjectMethod(env, map, put, member, emptyHooks);
        (*env)->DeleteLocalRef(env, entry);
        (*env)->DeleteLocalRef(env, entryClass);
        (*env)->DeleteLocalRef(env, member);
        (*env)->DeleteLocalRef(env, hooks);
        if (!antied) {
            antied = true;
        }
    }

    (*env)->DeleteLocalRef(env, classIterator);
    (*env)->DeleteLocalRef(env, iterator);
    (*env)->DeleteLocalRef(env, classSet);
    (*env)->DeleteLocalRef(env, entrySet);
    (*env)->DeleteLocalRef(env, classMap);
    (*env)->DeleteLocalRef(env, map);

    return antied;
}

static inline void fill_java_lang_VMClassLoader(char v[]) {
    // java/lang/VMClassLoader
    static unsigned int m = 0;

    if (m == 0) {
        m = 19;
    } else if (m == 23) {
        m = 29;
    }

    v[0x0] = 'n';
    v[0x1] = 'd';
    v[0x2] = 'p';
    v[0x3] = 'f';
    v[0x4] = '\'';
    v[0x5] = 'e';
    v[0x6] = 'k';
    v[0x7] = 'e';
    v[0x8] = 'k';
    v[0x9] = '"';
    v[0xa] = 'X';
    v[0xb] = 'B';
    v[0xc] = 'S';
    v[0xd] = '}';
    v[0xe] = 's';
    v[0xf] = 's';
    v[0x10] = 'r';
    v[0x11] = 'N';
    v[0x12] = 'l';
    v[0x13] = 'e';
    v[0x14] = 'a';
    v[0x15] = 'c';
    v[0x16] = 'u';
    for (unsigned int i = 0; i < 0x17; ++i) {
        v[i] ^= ((i + 0x17) % m);
    }
    v[0x17] = '\0';
}

static inline void fill_findLoadedClass(char v[]) {
    // findLoadedClass
    static unsigned int m = 0;

    if (m == 0) {
        m = 13;
    } else if (m == 17) {
        m = 19;
    }

    v[0x0] = 'd';
    v[0x1] = 'j';
    v[0x2] = 'j';
    v[0x3] = 'a';
    v[0x4] = 'J';
    v[0x5] = 'h';
    v[0x6] = 'i';
    v[0x7] = 'm';
    v[0x8] = 'o';
    v[0x9] = 'o';
    v[0xa] = 'O';
    v[0xb] = 'l';
    v[0xc] = '`';
    v[0xd] = 'q';
    v[0xe] = 'p';
    for (unsigned int i = 0; i < 0xf; ++i) {
        v[i] ^= ((i + 0xf) % m);
    }
    v[0xf] = '\0';
}

static inline void fill_findLoadedClass_signature(char v[]) {
    // (Ljava/lang/ClassLoader;Ljava/lang/String;)Ljava/lang/Class;
    static unsigned int m = 0;

    if (m == 0) {
        m = 59;
    } else if (m == 61) {
        m = 67;
    }

    v[0x0] = ')';
    v[0x1] = 'N';
    v[0x2] = 'i';
    v[0x3] = 'e';
    v[0x4] = 's';
    v[0x5] = 'g';
    v[0x6] = '(';
    v[0x7] = 'd';
    v[0x8] = 'h';
    v[0x9] = 'd';
    v[0xa] = 'l';
    v[0xb] = '#';
    v[0xc] = 'N';
    v[0xd] = 'b';
    v[0xe] = 'n';
    v[0xf] = 'c';
    v[0x10] = 'b';
    v[0x11] = '^';
    v[0x12] = '|';
    v[0x13] = 'u';
    v[0x14] = 'q';
    v[0x15] = 's';
    v[0x16] = 'e';
    v[0x17] = '#';
    v[0x18] = 'U';
    v[0x19] = 'p';
    v[0x1a] = 'z';
    v[0x1b] = 'j';
    v[0x1c] = '|';
    v[0x1d] = '1';
    v[0x1e] = 's';
    v[0x1f] = 'A';
    v[0x20] = 'O';
    v[0x21] = 'E';
    v[0x22] = '\x0c';
    v[0x23] = 'w';
    v[0x24] = 'Q';
    v[0x25] = 'T';
    v[0x26] = 'N';
    v[0x27] = 'F';
    v[0x28] = 'N';
    v[0x29] = '\x11';
    v[0x2a] = '\x02';
    v[0x2b] = '`';
    v[0x2c] = 'G';
    v[0x2d] = 'O';
    v[0x2e] = 'Y';
    v[0x2f] = 'Q';
    v[0x30] = '\x1e';
    v[0x31] = '^';
    v[0x32] = 'R';
    v[0x33] = 'Z';
    v[0x34] = 'R';
    v[0x35] = '\x19';
    v[0x36] = 't';
    v[0x37] = 'T';
    v[0x38] = 'X';
    v[0x39] = 'I';
    v[0x3a] = 's';
    v[0x3b] = ':';
    for (unsigned int i = 0; i < 0x3c; ++i) {
        v[i] ^= ((i + 0x3c) % m);
    }
    v[0x3c] = '\0';
}

static inline void fill_DexposedBridge(char v[]) {
    // me.anr
    static unsigned int m = 0;

    if (m == 0) {
        m = 5;
    } else if (m == 7) {
        m = 11;
    }

    v[0x0] = 'l';
    v[0x1] = 'g';
    v[0x2] = '-';
    v[0x3] = 'e';
    v[0x4] = 'n';
    v[0x5] = 's';
    for (unsigned int i = 0; i < 0x6; ++i) {
        v[i] ^= ((i + 0x6) % m);
    }
    v[0x6] = '\0';
}

static inline void fill_exp824(char v[]) {
    // exp824
    static unsigned int m = 0;

    if (m == 0) {
        m = 5;
    } else if (m == 7) {
        m = 11;
    }

    v[0x0] = 'd';
    v[0x1] = 'z';
    v[0x2] = 's';
    v[0x3] = '<';
    v[0x4] = '2';
    v[0x5] = '5';
    for (unsigned int i = 0; i < 0x6; ++i) {
        v[i] ^= ((i + 0x6) % m);
    }
    v[0x6] = '\0';
}

bool antiEpic(JNIEnv *env, int sdk) {
    char v1[0x10], v2[0x3d];
    bool antied = false;

    fill_exp824(v1); // 0x7
    jobject classLoader = findClassLoader(env, (const char *) v1, sdk);
    if (classLoader == NULL) {
        return false;
    }

#ifdef DEBUG
    LOGI("found %s", v1);
#endif

    clearHandler(env, sdk);

    fill_java_lang_VMClassLoader(v2); // 0x18
    jclass vmClassLoader = (*env)->FindClass(env, v2);
    if (vmClassLoader == NULL) {
        (*env)->ExceptionClear(env);
        return false;
    }
    fill_findLoadedClass(v1); // 0x10
    fill_findLoadedClass_signature(v2); // 0x3d
    jmethodID method = (*env)->GetStaticMethodID(env, vmClassLoader, v1, v2);
    if (method == NULL) {
        (*env)->ExceptionClear(env);
        goto cleanVmClassLoader;
    }
    fill_DexposedBridge(v2); // 0x7
    jstring stringDexposedBridge = (*env)->NewStringUTF(env, v2);
    jobject classDexposedBridge = (*env)->CallStaticObjectMethod(env,
                                                                 vmClassLoader,
                                                                 method,
                                                                 classLoader,
                                                                 stringDexposedBridge);

    if ((*env)->ExceptionCheck(env)) {
#ifdef DEBUG
        (*env)->ExceptionDescribe(env);
#endif
        (*env)->ExceptionClear(env);
    }

#ifdef DEBUG
    debug(env, "DexposedBridge: %s", classDexposedBridge);
#endif

    if (classDexposedBridge != NULL) {
        antied = doAntiEpic(env, (jclass) classDexposedBridge);
        (*env)->DeleteLocalRef(env, classDexposedBridge);
    }

    (*env)->DeleteLocalRef(env, stringDexposedBridge);
cleanVmClassLoader:
    (*env)->DeleteLocalRef(env, vmClassLoader);

    return antied;
}

#endif

static inline void fill_java_lang_Thread(char v[]) {
    // java/lang/Thread
    static unsigned int m = 0;
    if (m == 0) {
        m = 13;
    } else if (m == 17) {
        m = 19;
    }
    v[0x0] = 'i';
    v[0x1] = 'e';
    v[0x2] = 's';
    v[0x3] = 'g';
    v[0x4] = '(';
    v[0x5] = 'd';
    v[0x6] = 'h';
    v[0x7] = 'd';
    v[0x8] = 'l';
    v[0x9] = '#';
    v[0xa] = 'T';
    v[0xb] = 'i';
    v[0xc] = 'p';
    v[0xd] = 'f';
    v[0xe] = 'e';
    v[0xf] = 'a';
    for (unsigned int i = 0; i < 0x10; ++i) {
        v[i] ^= ((i + 0x10) % m);
    }
    v[0x10] = '\0';
}

static inline void fill_setUncaughtExceptionPreHandler(char v[]) {
    // setUncaughtExceptionPreHandler
    static unsigned int m = 0;

    if (m == 0) {
        m = 29;
    } else if (m == 31) {
        m = 37;
    }

    v[0x0] = 'r';
    v[0x1] = 'g';
    v[0x2] = 'w';
    v[0x3] = 'Q';
    v[0x4] = 'k';
    v[0x5] = 'e';
    v[0x6] = 'f';
    v[0x7] = '}';
    v[0x8] = 'n';
    v[0x9] = 'b';
    v[0xa] = '\x7f';
    v[0xb] = 'I';
    v[0xc] = 'u';
    v[0xd] = 'm';
    v[0xe] = 'j';
    v[0xf] = '`';
    v[0x10] = 'e';
    v[0x11] = '{';
    v[0x12] = '|';
    v[0x13] = 'z';
    v[0x14] = 'E';
    v[0x15] = 'd';
    v[0x16] = 'r';
    v[0x17] = 'P';
    v[0x18] = 'x';
    v[0x19] = 't';
    v[0x1a] = '\x7f';
    v[0x1b] = 'p';
    v[0x1c] = 'e';
    v[0x1d] = 's';
    for (unsigned int i = 0; i < 0x1e; ++i) {
        v[i] ^= ((i + 0x1e) % m);
    }
    v[0x1e] = '\0';
}

static inline void fill_setUncaughtExceptionHandler_signature(char v[]) {
    // (Ljava/lang/Thread$UncaughtExceptionHandler;)V
    static unsigned int m = 0;

    if (m == 0) {
        m = 43;
    } else if (m == 47) {
        m = 53;
    }

    v[0x0] = '+';
    v[0x1] = 'H';
    v[0x2] = 'o';
    v[0x3] = 'g';
    v[0x4] = 'q';
    v[0x5] = 'i';
    v[0x6] = '&';
    v[0x7] = 'f';
    v[0x8] = 'j';
    v[0x9] = 'b';
    v[0xa] = 'j';
    v[0xb] = '!';
    v[0xc] = '[';
    v[0xd] = 'x';
    v[0xe] = 'c';
    v[0xf] = 'w';
    v[0x10] = 'r';
    v[0x11] = 'p';
    v[0x12] = '1';
    v[0x13] = 'C';
    v[0x14] = 'y';
    v[0x15] = '{';
    v[0x16] = 'x';
    v[0x17] = 'o';
    v[0x18] = '|';
    v[0x19] = 't';
    v[0x1a] = 'i';
    v[0x1b] = '[';
    v[0x1c] = 'g';
    v[0x1d] = 'C';
    v[0x1e] = 'D';
    v[0x1f] = 'R';
    v[0x20] = 'W';
    v[0x21] = 'M';
    v[0x22] = 'J';
    v[0x23] = 'H';
    v[0x24] = 'o';
    v[0x25] = 'I';
    v[0x26] = 'G';
    v[0x27] = 'N';
    v[0x28] = 'l';
    v[0x29] = 'd';
    v[0x2a] = 'p';
    v[0x2b] = '8';
    v[0x2c] = '-';
    v[0x2d] = 'S';
    for (unsigned int i = 0; i < 0x2e; ++i) {
        v[i] ^= ((i + 0x2e) % m);
    }
    v[0x2e] = '\0';
}

void clearHandler(JNIEnv *env, int sdk) {
    char v1[0x1f], v2[0x2f];
    if (sdk < 26) {
        return;
    }
    fill_java_lang_Thread(v1); // 0x10 + 1
    jclass clazz = (*env)->FindClass(env, v1);
    fill_setUncaughtExceptionPreHandler(v1); // 0x1e + 1
    fill_setUncaughtExceptionHandler_signature(v2); // 0x2e + 1
    jmethodID method = (*env)->GetStaticMethodID(env, clazz, v1, v2);
    if (method == NULL) {
        (*env)->ExceptionClear(env);
    } else {
        (*env)->CallStaticVoidMethod(env, clazz, method, NULL);
    }
    (*env)->DeleteLocalRef(env, clazz);
}
