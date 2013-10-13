
UTILS=../utils

OBJDIR=obj/
CFLAGS := -g -Wall -O0 -I. -I$(UTILS) -Ifreetype-gl
CXXFLAGS=-std=c++0x
TARGET=grappix

LINUX_LIBS := -lglfw -lGL -lGLEW
OBJS := main.o shader.o basic_buffer.o renderbuffer.o image.o distancefield.o
OBJS += freetype-gl/texture-atlas.o freetype-gl/texture-font.o freetype-gl/vector.o freetype-gl/edtaa3func.o
MODULES := $(UTILS)/coreutils

ADK=/home/sasq/android
SDK=/opt/android-sdk-linux
APP_PLATFORM=android-10
MY_PACKAGE=grappix

EXT=.so
LDFLAGS := -shared -fPIC

LINUX_OBJS := window.o
ANDROID_OBJS := window_android.o android_native_app_glue.o
ANDROID_CFLAGS := -I$(ADK)/include -I$(ADK)/include/freetype2
ANDROID_LDFLAGS := -L$(ADK)/lib
ANDROID_LIBS := -llog -lpng -lfreetype -landroid -lEGL -lGLESv2 

LINUX_CFLAGS += `freetype-config --cflags`
LINUX_LIBS += `freetype-config --libs`
LINUX_CFLAGS += `libpng-config --cflags`
LINUX_LIBS += `libpng-config --libs`

LINUX_CC=ccache clang -Qunused-arguments
LINUX_CXX=ccache clang++ -Qunused-arguments

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