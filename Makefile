
UTILS=../utils

OBJDIR=obj/
CFLAGS := -g -Wall -O0 -I. -I$(UTILS) -Ifreetype-gl -DWITH_FREETYPE
CXXFLAGS=-std=c++0x
TARGET=grappix

LINUX_LIBS := -lglfw -lGL -lGLEW
OBJS := tiletest.o tiles.o shader.o basic_buffer.o renderbuffer.o
OBJS += tween.o image.o 
OBJS += distancefield.o freetype-gl/texture-atlas.o freetype-gl/texture-font.o freetype-gl/vector.o freetype-gl/edtaa3func.o
MODULES := $(UTILS)/coreutils

LINUX_OBJS := window.o
LINUX_CFLAGS += `freetype-config --cflags`
LINUX_LIBS += `freetype-config --libs`
LINUX_CFLAGS += `libpng-config --cflags`
LINUX_LIBS += `libpng-config --libs`

LINUX_CC=ccache clang -Qunused-arguments
LINUX_CXX=ccache clang++ -Qunused-arguments

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