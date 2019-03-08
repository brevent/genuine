LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := genuine
LOCAL_CFLAGS += -Wall -Wextra -Wno-format-security
LOCAL_LDFLAGS := -Wl,--hash-style=both
LOCAL_SRC_FILES := genuine.c genuine_extra.c plt.c
LOCAL_SRC_FILES += anti-xposed.c apk-sign-v2.c
LOCAL_SRC_FILES += epic.c libraries.cpp libraries-no-stl.c
LOCAL_LDLIBS    := -llog
include $(BUILD_SHARED_LIBRARY)
