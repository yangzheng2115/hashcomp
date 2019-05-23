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

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    find_package(Boost 1.50.0 REQUIRED COMPONENTS date_time filesystem iostreams)
    include_directories(${Boost_INCLUDE_DIR})
    link_directories(${Boost_LIBRARY_DIR})
    link_directories(/usr/local/lib)
endif ()

if (NOT CMAKE_SYSTEM_NAME MATCHES "CYGWIN")
    find_package(tbb 1.0 REQUIRED COMPONENTS tbbmalloc tbb tbb_preview)
    if (NOT TBB_FIND_COMPONENTS EQUAL 0)
        message("-- Found TBB at: ${TBB_DIR}")
        message("--   ${TBB_FIND_COMPONENTS} found at ${TBB_ROOT}")
        set(LTBB "-L${TBB_DIR}/../../ -ltbb")
    endif ()
endif ()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(PTHREAD -lpthread)
    set(MLIB -lm)
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
        )

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

set(HF_TESTS_EXES ${FH_TEST})
foreach (sourcefile ${HF_TESTS_EXES})
    get_filename_component(exename ${sourcefile} NAME_WE)
    add_executable(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} ${sourcefile} ${FH_LIBS})
    set_target_properties(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX}
            PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_RELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_MINRELEASE 1
            EXCLUDE_FROM_DEFAULT_BUILD_RELWITHDEBINFO 1
            OUTPUT_NAME ${exename}${ARTIFACT_SUFFIX}
            )
    target_link_libraries(${CMAKE_PROJECT_NAME}_${exename}${ARTIFACT_SUFFIX} ${PTHREAD})
endforeach (sourcefile ${FH_TESTS_EXES})
