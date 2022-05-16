LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := genuine
LOCAL_CFLAGS += -Oz -Wall -Wextra -Wshadow -Werror -fvisibility=hidden
LOCAL_LDFLAGS := -Wl,--hash-style=both
LOCAL_SRC_FILES := genuine.c genuine_extra.c plt.c inline.c
LOCAL_SRC_FILES += openat.c
LOCAL_SRC_FILES += am-proxy.c pm.c
LOCAL_SRC_FILES += anti-xposed.c apk-sign-v2.c
LOCAL_SRC_FILES += epic.c classloader.cpp hash.c
LOCAL_SRC_FILES += epic-method.c epic-field.c
LOCAL_SRC_FILES += dex-path-list.cpp
LOCAL_SRC_FILES += xposed-nop.cpp
LOCAL_SRC_FILES += common.c handle-error.c native-activity.c bitmap.c
LOCAL_SRC_FILES += path.c
ifeq (,$(wildcard $(LOCAL_PATH)/genuine.h))
$(warning "no genuine.h, won't check genuine")
endif
LOCAL_LDLIBS    := -llog -landroid -ljnigraphics -lz
include $(BUILD_SHARED_LIBRARY)
