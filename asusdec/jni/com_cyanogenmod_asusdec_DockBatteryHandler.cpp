/*
 * Copyright (C) 2013 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "AsusdecDockBatteryHandler"

#include "JNIHelp.h"
#include "jni.h"
#include <utils/Log.h>
#include <utils/misc.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/ioctl.h>



#include <linux/ioctl.h>

namespace asusdec {

#define POWER_SUPPLY_PATH "/sys/class/power_supply"

struct FieldIds {
    // members
    jfieldID mDockBatteryStatus;
    jfieldID mDockBatteryLevel;
    jfieldID mDockBatteryPresent;
    jfieldID mPadUsbOnline;
    jfieldID mAcOnline;
    jfieldID mDockAcOnline;
};
static FieldIds gFieldIds;

struct BatteryManagerConstants {
    jint dockStatusUnknown;
    jint dockStatusCharging;
    jint dockStatusNotCharging;
};
static BatteryManagerConstants gConstants;

struct PowerSupplyPaths {
    char* dockBatteryStatusPath;
    char* dockBatteryCapacityPath;
    char* dockBatteryPresentPath;
    char* padUsbOnlinePath;
    char* acOnlinePath;
    char* dockAcOnlinePath;
};
static PowerSupplyPaths gPaths;

static jint getDockBatteryStatus(const char* status)
{
    switch (status[0]) {
        case 'C': return gConstants.dockStatusCharging;         // Charging
        case 'N': return gConstants.dockStatusNotCharging;      // Not charging

        default: {
            ALOGW("Unknown dock battery status '%s'", status);
            return gConstants.dockStatusUnknown;
        }
    }
}

static int readFromFile(const char* path, char* buf, size_t size)
{
    if (!path)
        return -1;
    int fd = open(path, O_RDONLY, 0);
    if (fd == -1) {
        ALOGE("Could not open '%s'", path);
        return -1;
    }

    ssize_t count = read(fd, buf, size);
    if (count > 0) {
        while (count > 0 && buf[count-1] == '\n')
            count--;
        buf[count] = '\0';
    } else {
        buf[0] = '\0';
    }

    close(fd);
    return count;
}

static void setBooleanField(JNIEnv* env, jobject obj, const char* path, jfieldID fieldID)
{
    const int SIZE = 16;
    char buf[SIZE];

    jboolean value = false;
    if (readFromFile(path, buf, SIZE) > 0) {
        if (buf[0] != '0') {
            value = true;
        }
    }
    env->SetBooleanField(obj, fieldID, value);
}

static void setIntField(JNIEnv* env, jobject obj, const char* path, jfieldID fieldID, int def)
{
    const int SIZE = 128;
    char buf[SIZE];

    jint value = def;
    if (readFromFile(path, buf, SIZE) > 0) {
        value = atoi(buf);
    }
    env->SetIntField(obj, fieldID, value);
}

static void asusdec_DockBatteryHandler_nativeDockBatteryUpdate(JNIEnv* env, jobject obj)
{
    const int SIZE = 128;
    char buf[SIZE];

    // Level
    setIntField(env, obj, gPaths.dockBatteryCapacityPath, gFieldIds.mDockBatteryLevel, 0);
    // Status
    if (readFromFile(gPaths.dockBatteryStatusPath, buf, SIZE) > 0) {
        env->SetIntField(obj, gFieldIds.mDockBatteryStatus, getDockBatteryStatus(buf));
    }
    else {
        env->SetIntField(obj, gFieldIds.mDockBatteryStatus, gConstants.dockStatusUnknown);
    }
    // Present
    jboolean present = false;
    if (readFromFile(gPaths.dockBatteryPresentPath, buf, SIZE) >= 15) {
        // should return "dock detect = 1"
        if (buf[14] == '1') {
            present = true;
        }
    }
    env->SetBooleanField(obj, gFieldIds.mDockBatteryPresent, present);
    // PadUsbOnline
    setBooleanField(env, obj, gPaths.padUsbOnlinePath, gFieldIds.mPadUsbOnline);
    // AcOnline
    setBooleanField(env, obj, gPaths.acOnlinePath, gFieldIds.mAcOnline);
    // DockAcOnline
    setBooleanField(env, obj, gPaths.dockAcOnlinePath, gFieldIds.mDockAcOnline);
}

static JNINativeMethod sMethods[] = {
     /* name, signature, funcPtr */
     {"nativeDockBatteryUpdate", "()V", (void*)asusdec_DockBatteryHandler_nativeDockBatteryUpdate},
};

int register_asusdec_DockBatteryHandler(JNIEnv* env)
{
    char    path[PATH_MAX];
    struct dirent* entry;

    DIR* dir = opendir(POWER_SUPPLY_PATH);
    if (dir == NULL) {
        ALOGE("Could not open %s\n", POWER_SUPPLY_PATH);
    } else {
        while ((entry = readdir(dir))) {
            const char* name = entry->d_name;

            // ignore "." and ".."
            if (name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0))) {
                continue;
            }

            char buf[20];
            // Look for "type" file in each subdirectory
            snprintf(path, sizeof(path), "%s/%s/type", POWER_SUPPLY_PATH, name);
            int length = readFromFile(path, buf, sizeof(buf));
            if (length > 0) {
                if (buf[length - 1] == '\n')
                    buf[length - 1] = 0;

                if (strcmp(buf, "Mains") == 0) {
                    snprintf(path, sizeof(path), "%s/%s/online", POWER_SUPPLY_PATH, name);
                    if (access(path, R_OK) == 0)
                        gPaths.acOnlinePath = strdup(path);
                }
                else if (strcmp(buf, "USB") == 0) {
                    snprintf(path, sizeof(path), "%s/%s/online", POWER_SUPPLY_PATH, name);
                    if (access(path, R_OK) == 0)
                        gPaths.padUsbOnlinePath = strdup(path);
                }
                else if(strcmp(buf, "DockBattery") == 0) {
                    snprintf(path, sizeof(path), "%s/%s/status", POWER_SUPPLY_PATH, name);
                    if (access(path, R_OK) == 0)
                        gPaths.dockBatteryStatusPath = strdup(path);
                    snprintf(path, sizeof(path), "%s/%s/capacity", POWER_SUPPLY_PATH, name);
                    if (access(path, R_OK) == 0)
                        gPaths.dockBatteryCapacityPath = strdup(path);
                    snprintf(path, sizeof(path), "%s/%s/device/ec_dock", POWER_SUPPLY_PATH, name);
                    if (access(path, R_OK) == 0)
                        gPaths.dockBatteryPresentPath = strdup(path);

                } else if(strcmp(buf, "DockAC") == 0) {
                    snprintf(path, sizeof(path), "%s/%s/online", POWER_SUPPLY_PATH, name);
                    if (access(path, R_OK) == 0)
                        gPaths.dockAcOnlinePath = strdup(path);
                }
            }
        }
        closedir(dir);
    }

    if (!gPaths.dockBatteryStatusPath)
        ALOGE("dockBatteryStatusPath not found");
    if (!gPaths.dockBatteryCapacityPath)
        ALOGE("dockBatteryCapacityPath not found");
    if (!gPaths.dockBatteryPresentPath)
        ALOGE("dockBatteryPresentPath not found");
    if (!gPaths.padUsbOnlinePath)
        ALOGE("padUsbOnlinePath not found");
    if (!gPaths.dockAcOnlinePath)
        ALOGE("dockAcOnlinePath not found");
    if (!gPaths.acOnlinePath)
        ALOGE("acOnlinePath not found");

    // Fields
    jclass clazz = env->FindClass("com/cyanogenmod/asusdec/DockBatteryHandler");
    if (clazz == NULL) {
        ALOGE("Can't find com/cyanogenmod/asusdec/DockBatteryHandler");
        return -1;
    }
    gFieldIds.mDockBatteryStatus = env->GetFieldID(clazz, "mDockBatteryStatus", "I");
    gFieldIds.mDockBatteryLevel = env->GetFieldID(clazz, "mDockBatteryLevel", "I");
    gFieldIds.mDockBatteryPresent = env->GetFieldID(clazz, "mDockBatteryPresent", "Z");
    gFieldIds.mPadUsbOnline = env->GetFieldID(clazz, "mPadUsbOnline", "Z");
    gFieldIds.mAcOnline = env->GetFieldID(clazz, "mAcOnline", "Z");
    gFieldIds.mDockAcOnline = env->GetFieldID(clazz, "mDockAcOnline", "Z");
    LOG_FATAL_IF(gFieldIds.mDockBatteryStatus == NULL,
            "Unable to find DockBatteryHandler.DOCK_BATTERY_STATUS_PATH");
    LOG_FATAL_IF(gFieldIds.mDockBatteryLevel == NULL,
            "Unable to find DockBatteryHandler.DOCK_BATTERY_LEVEL_PATH");
    LOG_FATAL_IF(gFieldIds.mDockBatteryPresent == NULL,
            "Unable to find DockBatteryHandler.DOCK_BATTERY_PRESENT_PATH");
    LOG_FATAL_IF(gFieldIds.mPadUsbOnline == NULL,
            "Unable to find DockBatteryHandler.PAD_USB_ONLINE_PATH");
    LOG_FATAL_IF(gFieldIds.mAcOnline == NULL,
            "Unable to find DockBatteryHandler.AC_ONLINE_PATH");
    LOG_FATAL_IF(gFieldIds.mDockAcOnline == NULL,
            "Unable to find DockBatteryHandler.DOCK_AC_ONLINE_PATH");

    // Constants
    clazz = env->FindClass("android/os/BatteryManager");
    if (clazz == NULL) {
        ALOGE("Can't find android/os/BatteryManager");
        return -1;
    }
    gConstants.dockStatusUnknown = env->GetStaticIntField(clazz,
            env->GetStaticFieldID(clazz, "BATTERY_STATUS_UNKNOWN", "I"));
    gConstants.dockStatusCharging = env->GetStaticIntField(clazz,
            env->GetStaticFieldID(clazz, "BATTERY_STATUS_CHARGING", "I"));
    gConstants.dockStatusNotCharging = env->GetStaticIntField(clazz,
            env->GetStaticFieldID(clazz, "BATTERY_STATUS_NOT_CHARGING", "I"));

    // Register native methods
    return jniRegisterNativeMethods(env, "com/cyanogenmod/asusdec/DockBatteryHandler", sMethods, NELEM(sMethods));
}

} /* namespace asusdec */
