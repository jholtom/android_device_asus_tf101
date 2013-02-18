#include <JNIHelp.h>
#include <android_runtime/AndroidRuntime.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define LOG_TAG "Asusdec-JNI"
#define ASUSDEC_DEV "/dev/asusdec"

// copied from drivers/input/asusec/asusdec.h
#define ASUSDEC_TP_ON   1
#define ASUSDEC_TP_OFF  0
#define ASUSDEC_IOC_MAGIC   0xf4
#define ASUSDEC_TP_CONTROL      _IOR(ASUSDEC_IOC_MAGIC, 5,  int)



// ----------------------------------------------------------------------------

/*
 * Class:     com_cyanogenmod_asusdec_KeyHandler
 * Method:    nativeToggleTouchpad
 * Signature: (Z)Z
 */
JNIEXPORT jboolean JNICALL Java_com_cyanogenmod_asusdec_KeyHandler_nativeToggleTouchpad
  (JNIEnv *env, jclass cls, jboolean status) {
    LOGD("Switching touchpad %d\n", status);

    int fd = open(ASUSDEC_DEV, O_RDONLY | O_NONBLOCK);

    if (fd < 0) {
        LOGE("Could  open device %s\n", ASUSDEC_DEV);
        return -1;
    }

    int on = (status == 0) ? ASUSDEC_TP_OFF : ASUSDEC_TP_ON;
    int success = ioctl(fd, ASUSDEC_TP_CONTROL, on);

    if (success != 0) {
        LOGE("Error calling ioctl, %d\n", success);
    }

    close(fd);

    LOGD("Touchpad is %d\n", on);
    return (jboolean) ((on == 1) ? true : false);
}


// ----------------------------------------------------------------------------

static const JNINativeMethod gMethods[] = {
        { "nativeToggleTouchpad", "(Z)Z", (void *) Java_com_cyanogenmod_asusdec_KeyHandler_nativeToggleTouchpad }
};


// ----------------------------------------------------------------------------

/*
 * This is called by the VM when the shared library is first loaded.
 */
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if(android::AndroidRuntime::registerNativeMethods(
            env, "com/cyanogenmod/asusdec/KeyHandler", gMethods,
            sizeof(gMethods) / sizeof(gMethods[0])) != JNI_OK) {
        LOGE("Failed to register native methods");
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

bail:
    return result;
}
