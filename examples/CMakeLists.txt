cmake_minimum_required(VERSION 3.12)

OPTION(MEMEORY_CHECK ON)

# def function
function(set_output_directory target)
    set(BIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)
    set_target_properties(${target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_OUTPUT_DIR}/Debug
            RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BIN_OUTPUT_DIR}/Release
            )
    if(MSVC AND MEMEORY_CHECK)
        target_compile_options(${target}
            "/Zi"  # Generate complete debug information
            "/MDd" # /MDd or /MTd to define _DEBUG and allow _CrtDumpMemoryLeaks()
            "/EHsc" # Specify the exception handling model - mode 'sc'
        )
        add_definitions(-DHAVE_MEMEORY_CHECK)
    endif()
endfunction()

function(add_executable_and_install target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} pHash)
    set_output_directory(${target_name})
    install(TARGETS ${target_name} DESTINATION bin)
endfunction()


add_compile_definitions(RES_DIR_PATH="${PROJECT_SOURCE_DIR}/resources")
include_directories(${PROJECT_SOURCE_DIR}/src)

set_output_directory(pHash)

if(HAVE_IMAGE_HASH)
    add_executable_and_install(TestFileImgHash imagehash-test-file.cpp)
    add_executable_and_install(TestCImgHash imagehash-test-cimg.cpp)
    add_executable_and_install(TestMultipthreadCImgHash imagehash-test-cimg-multipthread.cpp)
    add_executable_and_install(TestNoblurCImgHash imagehash-test-cimg-no-blur.cpp)

    if(PHASH_MVP)
        add_executable_and_install(TestMvptreeDct test_mvptree_dct.cpp)
        add_executable_and_install(TestMvptreeDct2 test_mvptree_dct2.cpp)
    endif()
endif()

# if(HAVE_IMAGE_HASH)
# add_executable(TestDCT "${EXAMPLEDIR}/test_imagephash.cpp")
# target_link_libraries (TestDCT pHash)
# install(TARGETS TestDCT DESTINATION bin)

# add_executable(TestRadish "${EXAMPLEDIR}/test_radish.cpp")
# target_link_libraries (TestRadish pHash)
# install(TARGETS TestRadish DESTINATION bin)

# add_executable(TestMH "${EXAMPLEDIR}/test_mhimagehash.cpp")
# target_link_libraries (TestMH pHash)
# install(TARGETS TestMH DESTINATION bin)
# endif()

if(HAVE_AUDIO_HASH)
    add_executable_and_install(TestAudio "${EXAMPLEDIR}/test_audiophash.cpp")
endif()

if(HAVE_VIDEO_HASH)
    add_executable_and_install(TestVideoHash "${EXAMPLEDIR}/test_dctvideohash.cpp")
endif()
