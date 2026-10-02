LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION := .cpp .cc

ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
    LOCAL_MODULE := JavascriptPlugins
else
    LOCAL_MODULE := JavascriptPlugins64
    LOCAL_CXXFLAGS += -DAML64
endif

rwildcard = $(foreach d,$(wildcard $(1)/*),$(call rwildcard,$(d),$(2)) $(filter $(subst *,%,$(2)),$(d)))

LOCAL_SRC_FILES := $(call rwildcard,mod,*.cpp)
LOCAL_SRC_FILES += $(call rwildcard,src,*.cpp)
LOCAL_SRC_FILES += $(call rwildcard,json,*.cpp)

LOCAL_SRC_FILES += \
    quickjs/quickjs.c \
    quickjs/dtoa.c \
    quickjs/libregexp.c \
    quickjs/libunicode.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)/quickjs

LOCAL_CFLAGS += -std=c11
LOCAL_CXXFLAGS += -std=c++17 -O2 -DNDEBUG

LOCAL_CXXFLAGS += -DOBFUSCATION_KEY=\"$(OBFUSCATION_KEY)\"

LOCAL_LDLIBS += -llog

include $(BUILD_SHARED_LIBRARY)