#------------------------
# ApplicationLib library

#set local path
LOCAL_PATH := $(call my-dir)

MY_PROJECT_ROOT := $(LOCAL_PATH)/../..
DAVA_ROOT := $(MY_PROJECT_ROOT)/../..

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := UnitTestsLib

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Classes
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Classes/Infrastructure
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../Sources/Tools

# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

# set source files
LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/../../Classes/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../Classes/Infrastructure/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../Classes/SFML/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../Classes/SFML/Network/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../Classes/SFML/Network/Unix/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../Classes/SFML/System/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../../../Sources/Tools/TeamcityOutput/*.cpp) \
	$(wildcard $(LOCAL_PATH)/../../../../Sources/Internal/Platform/TemplateAndroid/ExternC/*.cpp) )

LOCAL_LDLIBS := -lz -lOpenSLES -landroid

ifneq ($(filter $(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard),)
LOCAL_ARM_NEON := true
LOCAL_NEON_CFLAGS := -mfloat-abi=softfp -mfpu=neon -march=armv7
endif

# set included libraries
LOCAL_STATIC_LIBRARIES := libInternal

# build shared library
include $(BUILD_SHARED_LIBRARY)

# include modules
$(call import-add-path,$(DAVA_ROOT)/Sources)
$(call import-module,Internal)