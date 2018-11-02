LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := genuine
LOCAL_CFLAGS += -Wall -Wextra -Werror
LOCAL_SRC_FILES := genuine.cpp
include $(BUILD_SHARED_LIBRARY)
