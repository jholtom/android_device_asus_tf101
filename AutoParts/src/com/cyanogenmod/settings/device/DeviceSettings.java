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

package com.cyanogenmod.settings.device;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Locale;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.PowerManager;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.provider.Settings;
import android.util.Log;
import android.os.SystemProperties;

import com.cyanogenmod.asusdec.KeyHandler;

public class DeviceSettings extends PreferenceActivity implements
        Preference.OnPreferenceChangeListener {
    private static final String TAG = "DeviceSettings";

    public static final String L10N_PREFIX = "asusdec,asusdec-";

    private static final String PREFS_FILE = "device_settings";
    private static final String PREFS_LANG = "lang";
    private static final String PREFS_TOUCHPAD_STATUS = "touchpad_status";
    private static final String PREFERENCE_KEYBOARD_LAYOUT = "keyboard_layout";
    private static final String PREFERENCE_CPU_MODE = "cpu_settings";
    private static final String CPU_PROPERTY = "sys.cpu.mode";

    private Context mContext;
    private ListPreference mKeyboardLayout;
    private ListPreference mCpuMode;

    @Override
    @SuppressWarnings("deprecation")
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.preferences);

        mContext = getApplicationContext();

        mKeyboardLayout = (ListPreference) getPreferenceScreen().findPreference(
                PREFERENCE_KEYBOARD_LAYOUT);
        mKeyboardLayout.setOnPreferenceChangeListener(this);
        setLayoutPreferenceValue();

        String mCurrCpuMode = "1";

        if (SystemProperties.get(CPU_PROPERTY) != null)
            mCurrCpuMode = SystemProperties.get(CPU_PROPERTY);

        mCpuMode = (ListPreference) getPreferenceScreen().findPreference(
                PREFERENCE_CPU_MODE);

        mCpuMode.setValueIndex(getCpuModeOffset(mCurrCpuMode));
        mCpuMode.setOnPreferenceChangeListener(this);
    }

    private void setLayoutPreferenceValue() {
        String layout = getLayoutPreference(mContext);
        CharSequence[] values = mKeyboardLayout.getEntryValues();

        for (int i = 0; i < values.length; i++) {
            if (layout.equals(values[i])) {
                mKeyboardLayout.setValue(layout);
                return;
            }
        }
    }

    private int getCpuModeOffset(String mode) {
        if (mode.equals("0")) {
            return 0;
        } else if (mode.equals("2")) {
            return 2;
        } else {
            return 1;
        }
    }

    public boolean onPreferenceChange(Preference preference, Object value) {
        if (preference.equals(mKeyboardLayout)) {
            final String newLanguage = (String) value;
            setNewKeyboardLanguage(mContext, newLanguage);

        } else if (preference.equals(mCpuMode)) {
            final String newCpuMode = (String) value;
            SystemProperties.set(CPU_PROPERTY, newCpuMode);
        }

        return true;
    }

    private static void setNewKeyboardLanguage(Context context, String language) {
        Log.d(TAG, "Setting new keyboard layout to l10n variant " + language);

        SharedPreferences prefs = context.getSharedPreferences(PREFS_FILE,
                Context.MODE_WORLD_READABLE);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putString(PREFS_LANG, language);
        editor.commit();

        String layout = L10N_PREFIX + language;
        Settings.System.putString(context.getContentResolver(),
                Settings.System.KEYLAYOUT_OVERRIDES, layout);
    }

    private static String getLayoutPreference(Context context) {
        SharedPreferences prefs = context.getSharedPreferences(PREFS_FILE,
                Context.MODE_WORLD_READABLE);
        String layout = prefs.getString(PREFS_LANG, "");

        if (layout.equals("")) {
            layout = Locale.getDefault().toString();
            Log.d(TAG, "Using default locale " + layout + " as keyboard layout");
        }

        return layout;
    }

    public static class BootCompletedReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction() != Intent.ACTION_BOOT_COMPLETED) {
                return;
            }
            String layout = getLayoutPreference(context);
            setNewKeyboardLanguage(context, layout);

            SharedPreferences prefs = context.getSharedPreferences(PREFS_FILE,
                    Context.MODE_WORLD_READABLE);
            boolean tpEnabled = prefs.getBoolean(PREFS_TOUCHPAD_STATUS, true);
            if (!tpEnabled) {
                new KeyHandler(context).enableTouchpad(false);
            }
        }
    }
}
