LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    com_cyanogenmod_asusdec_KeyHandler.cpp \
    com_cyanogenmod_asusdec_DockBatteryHandler.cpp \
    com_cyanogenmod_asusdec_AsusdecNative.cpp

LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE)

LOCAL_SHARED_LIBRARIES := \
    libandroid_runtime \
    libnativehelper \
    libcutils \
    libutils

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= libasusdec_jni

include $(BUILD_SHARED_LIBRARY)