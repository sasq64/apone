cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.musicplayer.uade)

SET(SOURCE_FILES
	src/unixatomic.c
	src/uadeipc.c
	src/amifilemagic.c
	src/eagleplayer.c
	src/unixwalkdir.c
	src/effects.c
	src/uadecontrol.c
	src/uadeconf.c
	src/uadestate.c
	src/uadeutils.c
	src/md5.c
	src/ossupport.c
	src/rmc.c
	src/songdb.c
	src/songinfo.c
	src/vparray.c
	src/support.c
	src/fifo.c
	src/newcpu.c
	src/memory.c
	src/custom.c
	src/cia.c
	src/audio.c
	src/compiler.c
	src/cpustbl.c
	src/missing.c
	src/sd-sound.c
	src/md-support.c
	src/cfgfile.c
	src/fpp.c
	src/debug.c
	src/readcpu.c
	src/cpudefs.c
	src/cpuemu.c
	src/uade.c
	src/uademain.c
	src/sinctable.c
	src/text_scope.c
	src/socketpair.c
	bencodetools/bencode.c
)

if(NOT APPLE)
	SET(SOURCE_FILES ${SOURCE_FILES} uade/compat/strlrep.c)
endif()

include_directories(include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer -Wall -Wno-unused -Wno-format -Wmissing-prototypes -Wstrict-prototypes -fno-exceptions")

add_library(uade ${SOURCE_FILES})



