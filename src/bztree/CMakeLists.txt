# No in-source build
cmake_minimum_required(VERSION 3.2.2)

include(ExternalProject)
set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
project(bztree)

# Set backend to PMDK by default to build persistent version. This will be passed
# in directly to PMwCAS which provides persistence support.
set(PMEM_BACKEND "PMDK" CACHE STRING "Persistent memory backend type")
string(TOUPPER ${PMEM_BACKEND} PMEM_BACKEND)
set(PMEM_BACKEND "PMDK")
set(GOOGLE_FRAMEWORK "OFF")

if (${PMEM_BACKEND} STREQUAL "PMDK")
    add_definitions(-DPMEM)
    add_definitions(-DPMDK)
    message(STATUS "Persistence support: PMDK")
elseif (${PMEM_BACKEND} STREQUAL "EMU")
    add_definitions(-DPMEM)
    add_definitions(-DPMEMEMU)
    message(STATUS "Persistence support: emulation")
elseif (${PMEM_BACKEND} STREQUAL "VOLATILE")
    message(STATUS "Persistence support: off")
else ()
    message(FATAL_ERROR "Unsupported persistent memory backend: ${PMEM_BACKEND}")
endif ()

##################### PMwCAS #########################
add_definitions(-DDESC_CAP=16)
set(PMWCAS_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/pmwcas")
ExternalProject_Add(PMWCAS
        PREFIX "${PMWCAS_PREFIX}"
        GIT_REPOSITORY https://github.com/HaoPatrick/pmwcas.git
        GIT_TAG master
        INSTALL_COMMAND ""
        BUILD_COMMAND $(MAKE)
        CMAKE_ARGS -DPMEM_BACKEND=${PMEM_BACKEND} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DDESC_CAP=16 -DBUILD_APPS=${BUILD_TESTS} -DGOOGLE_FRAMEWORK=${GOOGLE_FRAMEWORK}
        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        )

# PMwCAS and PMDK paths
include_directories(
        ${PMWCAS_PREFIX}/src/PMWCAS
        ${PMWCAS_PREFIX}/src/PMWCAS/src/
        ${PMWCAS_PREFIX}/src/PMWCAS/include
        ${PMWCAS_PREFIX}/src/PMWCAS-build/pmdk/src/PMDK/src/include
)
link_directories(${PMWCAS_PREFIX}/src/PMWCAS-build/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(LINK_FLAGS "-lnuma -lpthread -pthread -lrt -lpmwcas")
if (${PMEM_BACKEND} STREQUAL "PMDK")
    set(LINK_FLAGS "${LINK_FLAGS} -lpmemobj")
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        link_directories(${PMWCAS_PREFIX}/src/PMWCAS-build/pmdk/src/PMDK/src/debug)
    else ()
        link_directories(${PMWCAS_PREFIX}/src/PMWCAS-build/pmdk/src/PMDK/src/nondebug)
    endif ()
endif ()
link_libraries(${LINK_FLAGS})

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

message("########## ${CMAKE_BUILD_TYPE} ##########")
if (${CMAKE_BUILD_TYPE} STREQUAL "Profile")
    add_compile_definitions("PROFILE")
    link_libraries("-lprofiler")
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -march=native")
endif ()

set_property(GLOBAL APPEND PROPERTY BZTREE_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/bztree.cpp
        )
get_property(BZTREE_LIB_SRC GLOBAL PROPERTY BZTREE_SRC)

add_library(bztree SHARED ${BZTREE_LIB_SRC})
add_dependencies(bztree cpplint)
add_dependencies(bztree PMWCAS)

set(BZTREE_LINK_LIBS
        bztree
        )

add_custom_target(
        cpplint ALL
        COMMAND python2 ${CMAKE_CURRENT_SOURCE_DIR}/third-party/cpplint/cpplint.py
        --linelength=100
        --filter=-runtime/references,-build/header_guard,-build/include
        ${CMAKE_CURRENT_SOURCE_DIR}/bztree.h
        ${CMAKE_CURRENT_SOURCE_DIR}/bztree.cpp
        || (exit 0)
)

set(MAX_FREEZE_RETRY 1 CACHE STRING "MAX retry on frozen node")
target_compile_definitions(bztree PRIVATE MAX_FREEZE_RETRY=${MAX_FREEZE_RETRY})

set(ENABLE_MERGE 0 CACHE STRING "MAX retry on frozen node")
message(STATUS "ENABLE_MERGE: " ${ENABLE_MERGE})
target_compile_definitions(bztree PRIVATE ENABLE_MERGE=${ENABLE_MERGE})
