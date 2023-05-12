cmake_minimum_required(VERSION 3.12)

add_compile_definitions(RES_DIR_PATH="${PROJECT_SOURCE_DIR}/resources")
set(INSTALL_DIR ${PROJECT_SOURCE_DIR}/bin)
include_directories(${PROJECT_SOURCE_DIR}/src)

if (MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${INSTALL_DIR})
    set_target_properties(pHash PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${INSTALL_DIR}/Debug")
    set_target_properties(pHash PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${INSTALL_PATH}/Release")
endif (MSVC)

if(HAVE_IMAGE_HASH)
    add_executable(TestFileImgHash imagehash-test-file.cpp)
    target_link_libraries(TestFileImgHash pHash)
    install(TARGETS TestFileImgHash DESTINATION ${INSTALL_DIR})

    add_executable(TestCImgHash imagehash-test-cimg.cpp)
    target_link_libraries(TestCImgHash pHash)
    install(TARGETS TestCImgHash DESTINATION ${INSTALL_DIR})

    add_executable(TestMultipthreadCImgHash imagehash-test-cimg-multipthread.cpp)
    target_link_libraries(TestMultipthreadCImgHash pHash)
    install(TARGETS TestMultipthreadCImgHash DESTINATION ${INSTALL_DIR})

    add_executable(TestNoblurCImgHash imagehash-test-cimg-no-blur.cpp)
    target_link_libraries(TestNoblurCImgHash pHash)
    install(TARGETS TestNoblurCImgHash DESTINATION ${INSTALL_DIR})
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
add_executable(TestAudio "${EXAMPLEDIR}/test_audiophash.cpp")
target_link_libraries (TestAudio pHash)
install(TARGETS TestAudio DESTINATION bin)
endif()

if(HAVE_VIDEO_HASH)
add_executable(TestVideoHash "${EXAMPLEDIR}/test_dctvideohash.cpp")
target_link_libraries (TestVideoHash pHash)
install(TARGETS TestVideoHash DESTINATION bin)
endif()
