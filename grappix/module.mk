ifeq ($(GRAPPIX_INCLUDED),)
GRAPPIX_INCLUDED = 1
GRAPPIX_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

CPP_MODS := $(realpath $(CPP_MODS))
ifeq ($(CPP_MODS),)
  CPP_MODS := $(realpath ../cpp-mods)
endif

ifeq ($(CPP_MODS),)
  $(error Can not find 'cpp-mods'. Please set CPP_MODS to point to that directory)
endif

INCLUDES += $(GRAPPIX_DIR) $(GRAPPIX_DIR)grappix/freetype-gl
DIRS += $(GRAPPIX_DIR)grappix $(GRAPPIX_DIR)grappix/freetype-gl $(GRAPPIX_DIR)grappix/shaders
CFLAGS +=  -DWITH_FREETYPE

ifeq ($(HOST),android)
  FILES += $(GRAPPIX_DIR)grappix/specific/window_android.cpp $(GRAPPIX_DIR)grappix/specific/eglutil.cpp
  LIBS += -lz -llog -landroid -lEGL -lGLESv2
else ifeq ($(HOST),emscripten)
  FILES += $(GRAPPIX_DIR)grappix/specific/window_pc.cpp
else ifeq ($(HOST),apple)
  FILES += $(GRAPPIX_DIR)grappix/specific/window_pc.cpp
  LIBS += -lglfw -framework OpenGL -lGLEW
else ifeq ($(HOST),raspberrypi)
  VC := $(PI_SDK)/vc
  FILES += $(GRAPPIX_DIR)grappix/specific/window_pi.cpp $(GRAPPIX_DIR)grappix/specific/eglutil.cpp
  LIBS += -lz -lEGL -lGLESv2 -lvcos -lvchiq_arm -lbcm_host
  CFLAGS += -I$(VC)/include -I$(VC)/include/interface/vcos/pthreads -I$(VC)/include/interface/vmcs_host/linux

  # -I/opt/raspberry/include
  LDFLAGS += -L$(VC)/lib
  # -L/home/sasq/raspberrypi/arm
else
  FILES += $(GRAPPIX_DIR)grappix/specific/window_pc.cpp
  LIBS += -lglfw -lGL -lGLEW
endif

include $(CPP_MODS)/image/module.mk
include $(CPP_MODS)/tween/module.mk
include $(GRAPPIX_DIR)freetype/module.mk
include $(CPP_MODS)/coreutils/module.mk

endif