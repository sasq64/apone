cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.libz)

set(ZLIB_DIR zlib-1.2.8)

set(SOURCE_FILES
	${ZLIB_DIR}/adler32.c
	${ZLIB_DIR}/crc32.c
	${ZLIB_DIR}/deflate.c
	${ZLIB_DIR}/infback.c
	${ZLIB_DIR}/inffast.c
	${ZLIB_DIR}/inflate.c
	${ZLIB_DIR}/inftrees.c
	${ZLIB_DIR}/trees.c
	${ZLIB_DIR}/zutil.c
	${ZLIB_DIR}/compress.c
	${ZLIB_DIR}/uncompr.c
	${ZLIB_DIR}/gzclose.c
	${ZLIB_DIR}/gzlib.c
	${ZLIB_DIR}/gzread.c
	${ZLIB_DIR}/gzwrite.c
)

add_definitions(-D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN)

add_library(z ${SOURCE_FILES})
