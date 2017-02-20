# Android.mk
LOCAL_PATH := $(call my-dir)/../src
include $(CLEAR_VARS)

LOCAL_MODULE := FaceAlign
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(shell find $(LOCAL_PATH) -name "*.cpp"))
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include

LOCAL_CFLAGS += -O2 -fopenmp -DUSE_OPENMP -ffast-math -mfloat-abi=softfp -DANDROID
LOCAL_LDFLAGS += -fopenmp
LOCAL_CPPFLAGS += -std=c++11 -frtti -Wno-switch-enum -Wno-switch
LOCAL_LDLIBS := -llog -lm

include $(BUILD_SHARED_LIBRARY)
