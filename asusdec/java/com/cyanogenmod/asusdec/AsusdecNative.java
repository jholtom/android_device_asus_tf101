/*
 * Copyright (C) 2012 The CyanogenMod Project
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

import android.util.Log;

public final class AsusdecNative {

    private static final String TAG = "AsusdecNative";

    private static boolean sLoaded = false;

    public static synchronized void loadAsusdecLib() {
        if (!sLoaded) {
            System.loadLibrary("asusdec_jni");
            sLoaded = true;
            Log.i(TAG, "Asusdec native library loaded");
        }
    }

}
