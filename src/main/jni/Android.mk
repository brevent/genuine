LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := genuine
LOCAL_CFLAGS += -Oz -Wall -Wextra -Wshadow -Werror -fvisibility=hidden
LOCAL_LDFLAGS := -Wl,--hash-style=both
LOCAL_SRC_FILES := genuine.c genuine_extra.c plt.c inline.c
LOCAL_SRC_FILES += openat.c
LOCAL_SRC_FILES += am-proxy.c pm.c
LOCAL_SRC_FILES += anti-xposed.c apk-sign-v2.c
LOCAL_SRC_FILES += epic.c libraries.cpp
LOCAL_SRC_FILES += libraries-no-stl.c libraries-mock.cpp
LOCAL_SRC_FILES += common.c handle-error.c native-activity.c bitmap.c
LOCAL_SRC_FILES += path.c
ifeq (,$(wildcard $(LOCAL_PATH)/genuine.h))
$(warning "no genuine.h, won't check genuine")
endif
LOCAL_LDLIBS    := -llog -landroid -ljnigraphics
include $(BUILD_SHARED_LIBRARY)
