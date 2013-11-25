
UTILS=../utils

OBJDIR := obj/

TARGET=grappix

CFLAGS := -Wall -O2 -I. -I$(UTILS) -Ifreetype-gl -DWITH_FREETYPE
CXXFLAGS=-std=c++0x

#MAINOBJ := main.o
#ifneq ($(TOCOMPILE),)
#MAINOBJ := $(patsubst %.cpp,%.o,$(TOCOMPILE))
#endif

MAIN_FILES = main.cpp snake.cpp tiletest.cpp bobs.cpp simple.cpp blur.cpp map.cpp
MAINOBJ := main.o

LINUX_LIBS := -lglfw -lGL -lGLEW
OBJS := tiles.o shader.o basic_buffer.o texture.o
OBJS += tween.o image.o 
OBJS += distancefield.o freetype-gl/texture-atlas.o freetype-gl/texture-font.o freetype-gl/vector.o freetype-gl/edtaa3func.o
MODULES := $(UTILS)/coreutils

SHADERS=$(patsubst %.glsl,%.o, $(wildcard shaders/*.glsl))
#SHADERS := $(addprefix $(OBJDIR),$(SHADERS))
#CGC := cgc
#XXD := xxd
OBJS += $(SHADERS)

LINUX_OBJS := window.o

EMROOT=/opt/emscripten

ifneq ($(EMSCRIPTEN),)
CFLAGS += -I$(EMROOT)/system/include/freetype2
LDFLAGS += --preload-file data --preload-file fonts
LIBS += -lfreetype -lz
#LDFLAGS += -s FULL_ES2=1
else
LINUX_CFLAGS += `freetype-config --cflags`
LINUX_LIBS += `freetype-config --libs`
LINUX_CFLAGS += `libpng-config --cflags`
LINUX_LIBS += `libpng-config --libs`
CC=ccache clang -Qunused-arguments
CXX=ccache clang++ -Qunused-arguments
endif

#LINUX_CC=ccache clang -Qunused-arguments
#LINUX_CXX=ccache clang++ -Qunused-arguments
#CXX=em++
#CC=emcc

ADK=/opt/arm-linux-androideabi
SDK=/opt/android-sdk-linux
APP_PLATFORM=android-10
MY_PACKAGE=grappix

#ANDROID_TARGET=libgrappix
ANDROID_EXT=.so
ANDROID_LDFLAGS := -fPIC -Wl,-soname,lib$(TARGET).so -Wl,-shared --sysroot=/opt/android-ndk-r9/platforms/android-14/arch-arm -no-canonical-prefixes -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now 
ANDROID_OBJS := android/window_android.o android/android_native_app_glue.o
ANDROID_CFLAGS := -Iandroid -I$(ADK)/include -I$(ADK)/include/freetype2
#ANDROID_LDFLAGS += -L.
#-L$(ADK)/lib
ANDROID_LIBS := $(ADK)/lib/libfreetype.a $(ADK)/lib/libpng.a -lz -llog -landroid -lEGL -lGLESv2
# -lpng
# -lpng -lfreetype

all : start_rule

.PHONY : shaders

shaders: $(SHADERS)
	echo $(SHADERS)

apkg :
	$(SDK)/build-tools/18.1.0/aapt package -f -M AndroidManifest.xml -S res -I $(SDK)/platforms/$(APP_PLATFORM)/android.jar -F temp.apk --generate-dependencies
	mkdir -p lib
	cp -a libs/* lib/
	zip -D -u -r temp.apk lib classes.dex
	rm -rf lib
	jarsigner -verbose -sigalg MD5withRSA -digestalg SHA1 -storepass android -keystore debug.keystore temp.apk androiddebugkey
	zipalign -f -v 4 temp.apk $(MY_PACKAGE).apk
	rm temp.apk


include $(UTILS)/Makefile.inc