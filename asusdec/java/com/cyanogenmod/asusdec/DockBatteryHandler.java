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

package com.cyanogenmod.asusdec;

import android.content.Context;
import android.os.BatteryManager;
import android.os.Bundle;

import com.android.internal.os.DeviceDockBatteryHandler;

public final class DockBatteryHandler implements DeviceDockBatteryHandler {
    private static final String TAG = "AsusdecDockBatteryHandler";

    private static final boolean DEBUG = false;

    private static final int DOCK_BATTERY_STATUS_UNKNOWN =
                                        BatteryManager.BATTERY_STATUS_UNKNOWN;
    private static final int DOCK_BATTERY_STATUS_CHARGING =
                                        BatteryManager.BATTERY_STATUS_CHARGING;
    private static final int DOCK_BATTERY_STATUS_NOT_CHARGING =
                                        BatteryManager.BATTERY_STATUS_NOT_CHARGING;

    private Context mContext;

    /* Begin native fields: All of these fields are set by native code. */
    private int mDockBatteryStatus;
    private int mDockBatteryLevel;
    private boolean mDockBatteryPresent;
    private int mDockBatteryPlugged;
    private boolean mPadUsbOnline;
    private boolean mAcOnline;
    private boolean mDockAcOnline;
    /* End native fields. */

    private boolean mInitial;
    private int mLastDockBatteryStatus;
    private int mLastDockBatteryLevel;
    private boolean mLastDockBatteryPresent;
    private int mLastDockBatteryPlugged;
    private boolean mLastPadUsbOnline;
    private boolean mLastAcOnline;
    private boolean mLastDockAcOnline;

    private Object mLock = new Object();
    private boolean mIgnoreUpdates = false;

    static {
        AsusdecNative.loadAsusdecLib();
    }

    public DockBatteryHandler(Context context) {
        mContext = context;
        mInitial = true;
    }

    @Override
    public void update() {
        nativeDockBatteryUpdate();
    }

    @Override
    public void process() {
        mDockBatteryPlugged = 0;
        if (this.mDockAcOnline || this.mAcOnline) {
            mDockBatteryPlugged = BatteryManager.BATTERY_PLUGGED_AC;
        } else if (this.mPadUsbOnline) {
            mDockBatteryPlugged = BatteryManager.BATTERY_PLUGGED_USB;
        }
    }

    public Bundle getNotifyData() {
        Bundle bundle = new Bundle();
        // Common data
        bundle.putInt(BatteryManager.EXTRA_DOCK_STATUS, this.mDockBatteryStatus);
        bundle.putInt(BatteryManager.EXTRA_DOCK_LEVEL, this.mDockBatteryLevel);
        bundle.putBoolean(BatteryManager.EXTRA_DOCK_PRESENT, this.mDockBatteryPresent);
        bundle.putInt(BatteryManager.EXTRA_DOCK_PLUGGED, this.mDockBatteryPlugged);

        // EEPAD data
        bundle.putBoolean("usb_wakeup", this.mPadUsbOnline);
        bundle.putBoolean("ac_online", this.mAcOnline);
        bundle.putBoolean("dock_ac_online", this.mDockAcOnline);

        return bundle;
    }

    @Override
    public boolean hasNewData() {
        // Has anything changed?
        boolean hasNewData =
            mInitial ||
            mDockBatteryLevel != mLastDockBatteryLevel ||
            mDockBatteryStatus != mLastDockBatteryStatus ||
            mDockBatteryPresent != mLastDockBatteryPresent ||
            mDockBatteryPlugged != mLastDockBatteryPlugged ||
            mPadUsbOnline != mLastPadUsbOnline ||
            mAcOnline != mLastAcOnline ||
            mDockAcOnline != mLastDockAcOnline;

        // Save data
        mInitial = false;
        mLastDockBatteryLevel = mDockBatteryLevel;
        mLastDockBatteryStatus = mDockBatteryStatus;
        mLastDockBatteryPresent = mDockBatteryPresent;
        mLastDockBatteryPlugged = mDockBatteryPlugged;
        mLastPadUsbOnline = mPadUsbOnline;
        mLastAcOnline = mAcOnline;
        mLastDockAcOnline = mDockAcOnline;

        return hasNewData;
    }

    @Override
    public boolean isPlugged() {
        return mDockBatteryPresent && mDockBatteryPlugged != 0;
    }

    private native void nativeDockBatteryUpdate();
}
