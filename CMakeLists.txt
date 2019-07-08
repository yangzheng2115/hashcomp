cmake_minimum_required(VERSION 3.6)
project(hashcomp)
enable_language(CXX)
enable_language(C)
enable_language(ASM)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_C_STANDARD 11)
if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.1)
    set(CMAKE_CXX_STANDARD 14)
else ()
    set(CMAKE_CXX_STANDARD 11)
endif ()

include_directories("./")
include_directories("./src")
include_directories("./src/bench")
include_directories("./src/levelhash")
include_directories("./src/faster")
include_directories("src/faster/misc")
include_directories("src/faster/io")
include_directories("src/faster/cc")
include_directories("src/faster/core")
include_directories("src/faster/memory")
include_directories("src/faster/api")
include_directories("src/mhash")
include_directories("src/mhash/util")
include_directories("src/mhash/epoch")
include_directories("utils")

if (CMAKE_SYSTEM_NAME MATCHES " Darwin")
    find_package(Boost 1.50.0 REQUIRED COMPONENTS date_time filesystem iostreams)
    include_directories(${Boost_INCLUDE_DIR})
    link_directories(${Boost_LIBRARY_DIR})
    link_directories(/usr/local/lib)
endif ()

if (NOT CMAKE_SYSTEM_NAME MATCHES "CYGWIN")
    find_package(tbb 1.0 REQUIRED COMPONENTS tbbmalloc tbb tbb_preview)
    if (NOT TBB_FIND_COMPONENTS EQUAL 0)
        message("-- Found TBB at: ${TBB_DIR}")
        message("-- ${TBB_FIND_COMPONENTS} found at ${TBB_ROOT}")
        set(LTBB "-L${TBB_DIR}/../../ -ltbb")
    endif ()
endif ()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(PTHREAD -lpthread)
    set(MLIB -lm)
    set(AIO -laio)
    set(UUID -luuid)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wimplicit-function-declaration")
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wincompatible-pointer-types -Wint-conversion")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wimplicit-function-declaration -Wdiscarded-qualifiers")
endif ()

set(LH_LIBS
        src/levelhash/hash.c
        src/levelhash/level_hashing.c
        )

set(FH_LIBS
        src/faster/misc/address.cc
        src/faster/misc/lss_allocator.cc
        src/faster/misc/thread.cc
        )
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(FH_LIBS
            src/faster/io/file_linux.cc
            ${FH_LIBS})
else ()
    set(FH_LIBS
            src/faster/io/file_windows.cc
            ${FH_LIBS})
endif ()

set(MH_LIBS
        src/mhash/epoch/faster/lss_allocator.cc
        src/mhash/epoch/faster/thread.cc)

option(WITH_LH_TESTS "build with lhests" ON)
if (WITH_LH_TESTS)
    set(LH_TEST
            tests/LH_MicroTest.cpp)
endif ()

option(WITH_FH_TESTS "build with fast hash tests" ON)
if (WITH_FH_TESTS)
    set(FH_TEST
            tests/FH_MicroTest.cpp)
endif ()

option(WITH_MH_TESTS "build with multi-level hash tests " ON)
if (WITH_MH_TESTS)
    set(MH_TEST
            tests/MH_MicroTest.cpp
            tests/MH_YCSB.cpp)
endif ()

option(WITH_UTILS "build with utils" ON)
if (WITH_UTILS)
    set(UTILS
            utils/DistributionTool.cpp
            utils/MicroDistTool.cpp)
endif ()

find_package(Threads)

set(LH_TESTS_EXES ${LH_TEST})
foreach (sourcefile ${LH_TESTS_EXES})
    get_filename_component(exename ${sourcefile} NAME_WE)
    add_executable(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} ${sourcefile} ${LH_LIBS})
    set_target_properties(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX}
            PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_RELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_MINRELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_RELWITHDEBINFO 1
            OUTPUT_NAME ${exename}${ARTIFACT_SUFFIX}
            )
    target_link_libraries(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} ${PTHREAD})
endforeach (sourcefile ${LH_TESTS_EXES})

set(FH_TESTS_EXES ${FH_TEST})
foreach (sourcefile ${FH_TESTS_EXES})
    get_filename_component(exename ${sourcefile} NAME_WE)
    add_executable(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} ${sourcefile} ${FH_LIBS})
    set_target_properties(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX}
            PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_RELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_MINRELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_RELWITHDEBINFO 1
            OUTPUT_NAME ${exename}${ARTIFACT_SUFFIX}
            )
    target_link_libraries(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} ${PTHREAD} ${LTBB} ${AIO} ${UUID})
endforeach (sourcefile ${FH_TESTS_EXES})

set(MF_TESTS_EXES ${MH_TEST})
foreach (sourcefile ${MF_TESTS_EXES})
    get_filename_component(exename ${sourcefile} NAME_WE)
    add_executable(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} ${sourcefile} ${MH_LIBS})
    set_target_properties(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX}
            PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_RELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_MINRELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_RELWITHDEBINFO 1
            OUTPUT_NAME ${exename}${ARTIFACT_SUFFIX}
            )
    target_link_libraries(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} ${PTHREAD})
endforeach (sourcefile ${MF_TESTS_EXES})

set(UTILS_EXES ${UTILS})
foreach (sourcefile ${UTILS_EXES})
    get_filename_component(exename ${sourcefile} NAME_WE)
    add_executable(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} ${sourcefile})
    set_target_properties(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX}
            PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_RELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_MINRELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_RELWITHDEBINFO 1
            OUTPUT_NAME ${exename}${ARTIFACT_SUFFIX}
            )
    target_link_libraries(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX})
endforeach (sourcefile ${UTILS_EXES})
