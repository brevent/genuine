//
// Created by Thom on 2019/2/16.
//

#include <jni.h>
#include <fcntl.h>
#include <unistd.h>
#include "plt.h"
#include "classloader.h"
#include "common.h"
#include "anti-xposed.h"
#include "epic.h"
#include "art.h"
#include <unordered_set>

static std::unordered_set<intptr_t> classLoaders;

static void doAntiXposed(JNIEnv *env, jobject object, intptr_t hash) {
    if (classLoaders.find(hash) != classLoaders.end()) {
        debug(env, "checked %s", object);
        return;
    }
    classLoaders.insert(hash);
#ifdef DEBUG
    LOGI("anti xposed, env: %p, class loader: %p", env, object);
#endif
#ifndef NO_CHECK_XPOSED_EDXPOSED
    if (doAntiEdXposed(env, object)) {
#ifdef DEBUG
        LOGI("antied edxposed");
#endif
    }
#endif
#ifdef CHECK_XPOSED_EPIC
    if (doAntiEpic(env, object)) {
#ifdef DEBUG
        LOGI("antied epic");
#endif
    }
#endif
}

static inline void fill_dalvik_system_PathClassLoader(char v[]) {
    // dalvik/system/PathClassLoader
    static unsigned int m = 0;

    if (m == 0) {
        m = 23;
    } else if (m == 29) {
        m = 31;
    }

    v[0x0] = 'b';
    v[0x1] = 'f';
    v[0x2] = 'd';
    v[0x3] = '\x7f';
    v[0x4] = 'c';
    v[0x5] = '`';
    v[0x6] = '#';
    v[0x7] = '~';
    v[0x8] = 'w';
    v[0x9] = '|';
    v[0xa] = 'd';
    v[0xb] = 't';
    v[0xc] = '\x7f';
    v[0xd] = '<';
    v[0xe] = 'D';
    v[0xf] = 't';
    v[0x10] = 'b';
    v[0x11] = 'h';
    v[0x12] = 'B';
    v[0x13] = 'n';
    v[0x14] = 'b';
    v[0x15] = 'w';
    v[0x16] = 'v';
    v[0x17] = 'J';
    v[0x18] = 'h';
    v[0x19] = 'i';
    v[0x1a] = 'm';
    v[0x1b] = 'o';
    v[0x1c] = 'y';
    for (unsigned int i = 0; i < 0x1d; ++i) {
        v[i] ^= ((i + 0x1d) % m);
    }
    v[0x1d] = '\0';
}

class PathClassLoaderVisitor : public art::SingleRootVisitor {
public:
    PathClassLoaderVisitor(JNIEnv *env, jclass classLoader) : env_(env), classLoader_(classLoader) {
    }

    void VisitRoot(art::mirror::Object *root, const art::RootInfo &info ATTRIBUTE_UNUSED) {
        jobject object = newLocalRef(env_, root);
        if (object != nullptr) {
            if (env_->IsInstanceOf(object, classLoader_)) {
                doAntiXposed(env_, object, (intptr_t) root);
            }
            DeleteLocalRef(env_, object);
        }
    }

private:
    JNIEnv *env_;
    jclass classLoader_;
};

static inline void fill_VisitRoots(char v[]) {
    // _ZN3art9JavaVMExt10VisitRootsEPNS_11RootVisitorE
    static unsigned int m = 0;

    if (m == 0) {
        m = 47;
    } else if (m == 53) {
        m = 59;
    }

    v[0x0] = '^';
    v[0x1] = 'X';
    v[0x2] = 'M';
    v[0x3] = '7';
    v[0x4] = 'd';
    v[0x5] = 't';
    v[0x6] = 's';
    v[0x7] = '1';
    v[0x8] = 'C';
    v[0x9] = 'k';
    v[0xa] = '}';
    v[0xb] = 'm';
    v[0xc] = '[';
    v[0xd] = 'C';
    v[0xe] = 'J';
    v[0xf] = 'h';
    v[0x10] = 'e';
    v[0x11] = '#';
    v[0x12] = '#';
    v[0x13] = 'B';
    v[0x14] = '|';
    v[0x15] = 'e';
    v[0x16] = '~';
    v[0x17] = 'l';
    v[0x18] = 'K';
    v[0x19] = 'u';
    v[0x1a] = 't';
    v[0x1b] = 'h';
    v[0x1c] = 'n';
    v[0x1d] = '[';
    v[0x1e] = 'O';
    v[0x1f] = 'n';
    v[0x20] = 'r';
    v[0x21] = '}';
    v[0x22] = '\x12';
    v[0x23] = '\x15';
    v[0x24] = 'w';
    v[0x25] = 'I';
    v[0x26] = 'H';
    v[0x27] = '\\';
    v[0x28] = '\x7f';
    v[0x29] = 'C';
    v[0x2a] = 'X';
    v[0x2b] = 'E';
    v[0x2c] = 'Y';
    v[0x2d] = 'A';
    v[0x2e] = 'r';
    v[0x2f] = 'D';
    for (unsigned int i = 0; i < 0x30; ++i) {
        v[i] ^= ((i + 0x30) % m);
    }
    v[0x30] = '\0';
}

static void checkGlobalRef(JNIEnv *env, jclass clazz) {
    char v[0x40];
    fill_VisitRoots(v);
    auto VisitRoots = (void (*)(void *, void *)) plt_dlsym(v, nullptr);
#ifdef DEBUG
    LOGI("VisitRoots: %p", VisitRoots);
#endif
    if (VisitRoots == nullptr) {
        return;
    }
    JavaVM *jvm;
    env->GetJavaVM(&jvm);
    PathClassLoaderVisitor visitor(env, clazz);
    VisitRoots(jvm, &visitor);
}

static inline void fill_SweepJniWeakGlobals(char v[]) {
    // _ZN3art9JavaVMExt19SweepJniWeakGlobalsEPNS_15IsMarkedVisitorE
    static unsigned int m = 0;

    if (m == 0) {
        m = 59;
    } else if (m == 61) {
        m = 67;
    }

    v[0x0] = ']';
    v[0x1] = 'Y';
    v[0x2] = 'J';
    v[0x3] = '6';
    v[0x4] = 'g';
    v[0x5] = 'u';
    v[0x6] = '|';
    v[0x7] = '0';
    v[0x8] = '@';
    v[0x9] = 'j';
    v[0xa] = 'z';
    v[0xb] = 'l';
    v[0xc] = 'X';
    v[0xd] = 'B';
    v[0xe] = 'U';
    v[0xf] = 'i';
    v[0x10] = 'f';
    v[0x11] = '"';
    v[0x12] = '-';
    v[0x13] = 'F';
    v[0x14] = 'a';
    v[0x15] = 'r';
    v[0x16] = '}';
    v[0x17] = 'i';
    v[0x18] = 'P';
    v[0x19] = 'u';
    v[0x1a] = 'u';
    v[0x1b] = 'J';
    v[0x1c] = '{';
    v[0x1d] = '~';
    v[0x1e] = 'K';
    v[0x1f] = 'f';
    v[0x20] = 'N';
    v[0x21] = 'L';
    v[0x22] = 'F';
    v[0x23] = 'D';
    v[0x24] = 'J';
    v[0x25] = 'T';
    v[0x26] = 'm';
    v[0x27] = 'y';
    v[0x28] = 'd';
    v[0x29] = 'x';
    v[0x2a] = 's';
    v[0x2b] = '\x1c';
    v[0x2c] = '\x1b';
    v[0x2d] = 'f';
    v[0x2e] = 'C';
    v[0x2f] = '|';
    v[0x30] = 'S';
    v[0x31] = 'A';
    v[0x32] = '_';
    v[0x33] = 'P';
    v[0x34] = 'R';
    v[0x35] = 'a';
    v[0x36] = 'Q';
    v[0x37] = 'J';
    v[0x38] = 'S';
    v[0x39] = 't';
    v[0x3a] = 'n';
    v[0x3b] = 'p';
    v[0x3c] = 'F';
    for (unsigned int i = 0; i < 0x3d; ++i) {
        v[i] ^= ((i + 0x3d) % m);
    }
    v[0x3d] = '\0';
}

class WeakClassLoaderVisitor : public art::IsMarkedVisitor {
public :
    WeakClassLoaderVisitor(JNIEnv *env, jclass classLoader) : env_(env), classLoader_(classLoader) {
    }

    art::mirror::Object *IsMarked(art::mirror::Object *obj) override {
        jobject object = newLocalRef(env_, obj);
        if (object != nullptr) {
            if (env_->IsInstanceOf(object, classLoader_)) {
                debug(env_, "visit weak root, instance: %s", object);
                doAntiXposed(env_, object, (intptr_t) obj);
            }
            DeleteLocalRef(env_, object);
        }
        return obj;
    }

private:
    JNIEnv *env_;
    jclass classLoader_;
};

static void checkWeakGlobalRef(JNIEnv *env, jclass clazz) {
    char v[0x40];
    fill_SweepJniWeakGlobals(v);
    auto SweepJniWeakGlobals = (void (*)(void *, void *)) plt_dlsym(v, nullptr);
#ifdef DEBUG
    LOGI("SweepJniWeakGlobals: %p", SweepJniWeakGlobals);
#endif
    if (SweepJniWeakGlobals == nullptr) {
        return;
    }
    JavaVM *jvm;
    env->GetJavaVM(&jvm);
    WeakClassLoaderVisitor visitor(env, clazz);
    SweepJniWeakGlobals(jvm, &visitor);
}

void checkClassLoader(JNIEnv *env, int sdk) {
    if (sdk < 21) {
        return;
    }

    char v[0x40];
    fill_dalvik_system_PathClassLoader(v);
    jclass clazz = env->FindClass(v);
    if (env->ExceptionCheck()) {
#ifdef DEBUG
        env->ExceptionDescribe();
#endif
        env->ExceptionClear();
    }
    if (clazz == nullptr) {
        return;
    }
    debug(env, "classLoader: %s", clazz);

    checkGlobalRef(env, clazz);
    checkWeakGlobalRef(env, clazz);

    classLoaders.clear();
    env->DeleteLocalRef(clazz);
}
