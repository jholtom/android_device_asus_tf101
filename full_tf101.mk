#
# Copyright (C) 2012 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# camera
PRODUCT_PACKAGES := \
	HoloSpiralWallpaper \
	LiveWallpapersPicker \
	VisualizationWallpapers \

PRODUCT_PACKAGES += \
	Camera

# Build asusdec
PRODUCT_PACKAGES += \
    com.cyanogenmod.asusdec \
    libasusdec_jni		
		
# AGPS patch
PRODUCT_COPY_FILES += \
    device/asus/tf101/prebuilt/etc/gps.conf:system/etc/gps.conf \
    device/asus/tf101/prebuilt/etc/SuplRootCert:system/etc/SuplRootCert \
    device/asus/tf101/prebuilt/etc/gps/gpsconfig.xml:system/etc/gps/gpsconfig.xml

# device
$(call inherit-product, device/asus/tf101/device.mk)

# inherit
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)

# product
PRODUCT_NAME := full_tf101
PRODUCT_DEVICE := tf101
PRODUCT_BRAND := asus
PRODUCT_MANUFACTURER := Asus
PRODUCT_MODEL := Transformer Pad


# AGPS patch
PRODUCT_COPY_FILES += \
    device/asus/tf101/prebuilt/etc/gps.conf:system/etc/gps.conf \
    device/asus/tf101/prebuilt/etc/SuplRootCert:system/etc/SuplRootCert \
    device/asus/tf101/prebuilt/etc/gps/gpsconfig.xml:system/etc/gps/gpsconfig.xml
