cmake_minimum_required(VERSION 3.12)

project(pHash)

OPTION(PHASH_DYNAMIC   "Build pHash dynamic library"                     ON)
OPTION(PHASH_STATIC    "Build pHash static library"                      OFF)
OPTION(PHASH_MVP       "Add MVP Hash Storage"                            OFF)
OPTION(PHASH_EXT       "Add multithread function for pHash"              OFF)
OPTION(WITH_OTHER_IMAGE_HASH "Other Image Libraries dependencies"        OFF)
OPTION(WITH_AUDIO_HASH "Audio hash support"                              OFF)
OPTION(WITH_VIDEO_HASH "Video hash support"                              OFF)
OPTION(PHASH_EXAMPLES  "Compile examples"                                ON)
OPTION(USE_OPENMP      "Use OpenMP, CIMG support"                        OFF)

set(HAVE_IMAGE_HASH 1)
add_definitions(-DHAVE_IMAGE_HASH)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if(WITH_AUDIO_HASH)
    set(HAVE_AUDIO_HASH 1)
    add_definitions(-DHAVE_AUDIO_HASH)
endif()

if(WITH_VIDEO_HASH)
    set(HAVE_VIDEO_HASH 1)
    add_definitions(-DHAVE_AUDIO_HASH)
endif()

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message("not set Install prefix, use default")
    SET(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/out CACHE PATH "Install prefix" FORCE)
ENDIF()
message("install path: ${CMAKE_INSTALL_PREFIX}")

include_directories(${PROJECT_SOURCE_DIR}/src)

# check file
include(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX("unistd.h" HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
    add_definitions(-DHAVE_UNISTD_H)
endif()

CHECK_INCLUDE_FILE_CXX("dirent.h" HAVE_DIRENT_H)
if(HAVE_DIRENT_H)
    message("check file: 'dirent.h' has found")
    add_definitions(-DHAVE_DIRENT_H)
endif()

if(NOT WIN32)
    set(DIRENT_FILE "")
    EXEC_PROGRAM(uname ARGS -m OUTPUT_VARIABLE BUILD_SYSTEM)
    EXEC_PROGRAM(uname ARGS -s OUTPUT_VARIABLE CMAKE_SYSTEM_NAME)

    # on testing
    set(PHASH_MVP OFF)
else()
    add_definitions("-D_EXPORTING")
endif()

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif()

if(WITH_OTHER_IMAGE_HASH)
    set(LIBS_DEPS png z jpeg tiff)
else()
    set(LIBS_DEPS "")
endif()

find_library(LIBMPG123 mpg123)
if(LIBMPG123)
    message("libmpg123 found at ${LIBMPG123}")
    set(HAVE_LIBMPG123 1)
    list(APPEND LIBS_DEPS ${LIBMPG123})
endif()

if(USE_OPENMP)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        message("check file: OpenMP has found")
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        list(APPEND LIBS_DEPS OpenMP::OpenMP_CXX)
    endif()
endif(USE_OPENMP)

file(GLOB SRC_LIST src/pHash.cpp src/bmbhash.cpp)

if(PHASH_MVP)
    include_directories(${PROJECT_SOURCE_DIR}/ext)
    list(APPEND SRC_LIST ext/pHash_mvp.cpp)
    install(FILES ext/pHash_mvp.h DESTINATION include)
    list(APPEND SRC_LIST ext/sqlite3/sqlite3.c)
endif()

if(PHASH_EXT)
    message("add phash_ext files")
    include_directories(${PROJECT_SOURCE_DIR}/ext)
    list(APPEND SRC_LIST ext/pHash_ext.cpp)
    install(FILES ext/pHash_ext.h DESTINATION include)
    add_definitions(-DHAVE_PHASH_EXT)
endif()

if(HAVE_AUDIO_HASH)
    list(APPEND SRC_LIST src/audiophash.cpp src/ph_fft.cpp)
    set(LIBS_DEPS ${LIBS_DEPS} sndfile samplerate vorbis vorbisenc ogg)
endif()

if(HAVE_VIDEO_HASH)
    list(APPEND SRC_LIST src/cimgffmpeg.cpp)
    set(LIBS_DEPS ${LIBS_DEPS} avcodec avformat swscale avutil)
endif()

set(PHASH_PREFIX "bin")
if(PHASH_DYNAMIC)
	add_library(pHash SHARED ${DIRENT_FILE} ${SRC_LIST})
endif()

if(PHASH_STATIC)
	add_library(pHash STATIC ${DIRENT_FILE} ${SRC_LIST})
    set(PHASH_PREFIX "lib")
endif()

target_link_libraries(pHash ${LIBS_DEPS})
install(TARGETS pHash DESTINATION ${PHASH_PREFIX})
install(FILES src/pHash.h src/CImg.h DESTINATION include)

if(PHASH_EXAMPLES)
    add_subdirectory(${PROJECT_SOURCE_DIR}/examples)
endif(PHASH_EXAMPLES)

if (MSVC)
     # Set the startup project .
    if (NOT ${CMAKE_VERSION} VERSION_LESS "3.6.0")
          set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    endif ()
endif (MSVC)
