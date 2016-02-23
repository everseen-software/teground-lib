cmake_minimum_required(VERSION 2.8)

project(TegroundLib)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

# define main dirs
get_filename_component(TEGROUND_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
set(TEGROUND_TEST_DIR ${TEGROUND_DIR}/tests)
if(USE_CPP11)
  message(STATUS "Enabling C++11")
  set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

# add open cv

if(WIN32)
  set(OpenCV_DIR $ENV{OPENCV_DIR}/../..)
else()
  set(OpenCV_DIR "/usr/lib/opencv")
endif()

find_package(OpenCV REQUIRED core highgui)
include_directories(${OpenCV_INCLUDE_DIRS})

# define the sources of the test
set(SOURCES
    ${TEGROUND_TEST_DIR}/src/testmain.cpp
    ${TEGROUND_TEST_DIR}/src/sequencetestcase.cpp
    ${TEGROUND_TEST_DIR}/src/segmenttracktestcase.cpp
    ${TEGROUND_TEST_DIR}/src/segmenttracktesttestcase.cpp
    ${TEGROUND_TEST_DIR}/src/testsuitedrawtestcase.cpp
    ${TEGROUND_DIR}/include/tgdatafile.h
    ${TEGROUND_DIR}/include/tgglobal.h
    ${TEGROUND_DIR}/include/tgsegment.h
    ${TEGROUND_DIR}/include/tgsegmenttrack.h
    ${TEGROUND_DIR}/include/tgsegmenttracktest.h
    ${TEGROUND_DIR}/include/tgsegmentassertionwriter.h
    ${TEGROUND_DIR}/include/tgtracktest.h
    ${TEGROUND_DIR}/include/tgtestsuite.h
    ${TEGROUND_DIR}/include/tgsequence.h
    ${TEGROUND_DIR}/include/tgtrack.h
    ${TEGROUND_DIR}/include/tgtrackheader.h
)

# configure the executable
include_directories(${TEGROUND_DIR}/include)
include_directories(${TEGROUND_TEST_DIR}/catch/include)
add_executable(TestTegroundLib ${SOURCES})

# configure unit tests via CTest
enable_testing()
add_test(NAME RunTests COMMAND TestTegroundLib)
add_dependencies(check TestTegroundLib)
target_link_libraries(TestTegroundLib ${OpenCV_LIBS})

