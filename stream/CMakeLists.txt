cmake_minimum_required(VERSION 3.6)
#project(stream)
#set(CMAKE_C_COMPILER "gcc")
#set(CMAKE_CXX_COMPILER "g++")
#set(CMAKE_C_STANDARD 11)
#if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.1)
#    set(CMAKE_CXX_STANDARD 14)
#else ()
#    set(CMAKE_CXX_STANDARD 11)
#endif ()
message(${CMAKE_CURRENT_BINARY_DIR})

include_directories(src/quantile)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../src/bench)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    enable_testing()
    find_package(gtest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})
else ()
    include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/../../third-party/gtest-1.7.0/fused-src)
    include_directories(SYSTEM ${PROJECT_SOURCE_DIR}../../third-party/gtest-1.7.0/fused-src/gtest)
    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/../../third-party/gtest-1.7.0/fused-src/gtest build)
endif ()

set(STREAM_EFFICIENCYTEST
        tests/GK_EfficiencyTest.cpp)

foreach (sourcefile ${STREAM_EFFICIENCYTEST})
    get_filename_component(exename ${sourcefile} NAME_WE)
    add_executable(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} ${sourcefile})
    set_target_properties(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX}
            PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_RELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_MINRELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_RELWITHDEBINFO 1
            OUTPUT_NAME ${exename}${ARTIFACT_SUFFIX}
            )
    target_link_libraries(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX})
endforeach (sourcefile ${STREAM_EFFICIENCYTEST})

#set(STREAM_UNITTEST
#        tests/GK_UnitTest.cpp)
#
#foreach (sourcefile ${STREAM_UNITTEST})
#    get_filename_component(exename ${sourcefile} NAME_WE)
#    add_executable(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} ${sourcefile})
#    set_target_properties(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX}
#            PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_RELEASE 1
#            EXCLUDE_FROM_DEFAULT_BUILD_MINRELEASE 1
#            EXCLUDE_FROM_DEFAULT_BUILD_RELWITHDEBINFO 1
#            OUTPUT_NAME ${exename}${ARTIFACT_SUFFIX}
#            )
#    target_link_libraries(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} gtest)
#endforeach (sourcefile ${STREAM_TEST})
