cmake_minimum_required(VERSION 3.12)

project(pHash)

OPTION(PHASH_DYNAMIC   "Build pHash dynamic library"                     ON)
OPTION(PHASH_STATIC    "Build pHash static library"                      OFF)
OPTION(PHASH_MVP       "Add MVP Hash Storage"                            ON)
OPTION(PHASH_EXT       "Add multithread function for pHash, need c++11"  ON)
OPTION(WITH_OTHER_IMAGE_HASH "Other Image Libraries dependencies"        OFF)
OPTION(WITH_AUDIO_HASH "Audio hash support"                              OFF)
OPTION(WITH_VIDEO_HASH "Video hash support"                              OFF)
OPTION(PHASH_EXAMPLES  "Compile examples"                                ON)

set(HAVE_IMAGE_HASH 1)
add_definitions(-DHAVE_IMAGE_HASH)

if(WITH_AUDIO_HASH)
    set(HAVE_AUDIO_HASH 1)
    add_definitions(-DHAVE_AUDIO_HASH)
endif()

if(WITH_VIDEO_HASH)
    set(HAVE_VIDEO_HASH 1)
    add_definitions(-DHAVE_AUDIO_HASH)
endif()

# CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/pHash.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/src/pHash.h)
set(INSTALL_DIR ${PROJECT_SOURCE_DIR}/out)
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
else()
    set(DIRENT_FILE "${PROJECT_SOURCE_DIR}/src/win/dirent.c")
endif()

CHECK_INCLUDE_FILE_CXX("pthread.h" HAVE_PTHREAD_H)
if(HAVE_DIRENT_H)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    if(Threads_FOUND)
        message("check file: 'pthread' has found")
        set(LIBS_DEPS ${LIBS_DEPS} Threads::Threads)
        add_definitions(-DHAVE_PTHREAD)
    endif()
endif()

if(NOT WIN32)
    set(MMAN_FILE "")
    set(DIRENT_FILE "")
    EXEC_PROGRAM(uname ARGS -m OUTPUT_VARIABLE BUILD_SYSTEM)
    EXEC_PROGRAM(uname ARGS -s OUTPUT_VARIABLE CMAKE_SYSTEM_NAME)

    # on testing
    set(PHASH_MVP OFF)

else()
    include_directories(${PROJECT_SOURCE_DIR}/src/win)
    set(MMAN_FILE "${PROJECT_SOURCE_DIR}/src/win/mman.cpp")
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

# find_package(OpenMP)
# if(OpenMP_CXX_FOUND)
#     message("check file: OpenMP has found")
#     set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#     set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#     # target_link_libraries(pHash PUBLIC OpenMP::OpenMP_CXX)
#     list(APPEND LIBS_DEPS OpenMP::OpenMP_CXX)
# endif()

file(GLOB SRC_LIST src/pHash.cpp src/bmbhash.cpp)

if(PHASH_MVP)
    include_directories(${PROJECT_SOURCE_DIR}/ext)
    list(APPEND SRC_LIST ext/pHash_mvp.cpp)
endif()

if(PHASH_EXT)
    message("add phash_ext files, use c++11 multithread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    include_directories(${PROJECT_SOURCE_DIR}/ext)
    list(APPEND SRC_LIST ext/pHash_ext.cpp)
endif()

if(HAVE_AUDIO_HASH)
    list(APPEND SRC_LIST src/audiophash.cpp src/ph_fft.cpp)
    set(LIBS_DEPS ${LIBS_DEPS} sndfile samplerate vorbis vorbisenc ogg)
endif()

if(HAVE_VIDEO_HASH)
    list(APPEND SRC_LIST src/cimgffmpeg.cpp)
    set(LIBS_DEPS ${LIBS_DEPS} avcodec avformat swscale avutil)
endif()

if(PHASH_DYNAMIC)
	add_library(pHash SHARED ${MMAN_FILE} ${DIRENT_FILE} ${SRC_LIST})
endif()

if(PHASH_STATIC)
	add_library(pHash STATIC ${MMAN_FILE} ${DIRENT_FILE} ${SRC_LIST})
endif()

target_link_libraries(pHash ${LIBS_DEPS})
install(TARGETS pHash DESTINATION ${PROJECT_SOURCE_DIR}/bin)

if(PHASH_EXAMPLES)
    add_subdirectory(${PROJECT_SOURCE_DIR}/examples)
endif(PHASH_EXAMPLES)

if (MSVC)
     # Set the startup project .
    if (NOT ${CMAKE_VERSION} VERSION_LESS "3.6.0")
          set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    endif ()
endif (MSVC)
