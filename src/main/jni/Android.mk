LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := genuine
LOCAL_CFLAGS += -Wall -Wextra -Werror -Wold-style-cast -Wformat-pedantic
LOCAL_LDFLAGS := -Wl,--hash-style=both
LOCAL_SRC_FILES := genuine.cpp genuine_extra.cpp plt.c
LOCAL_LDLIBS    := -llog
include $(BUILD_SHARED_LIBRARY)
