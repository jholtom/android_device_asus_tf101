
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libandroid_runtime
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
LOCAL_SRC_FILES := \
	stub.c 
	
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

