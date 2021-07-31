cmake_minimum_required(VERSION 3.3)
project("libzbar" VERSION 0.11.5 DESCRIPTION "Complete cross-platform ZBar cmake port")
if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/bin")
endif()

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(APPLE)
    set(WIN32   False)
    set(UNIX    False)
endif(APPLE)

if(WIN32)
    set(CMAKE_C_FLAGS_RELEASE "/O2 /TC /w")
    set(CMAKE_C_FLAGS_DEBUG "/Zi /TC /w")
elseif(UNIX OR ANDROID)
    set(CMAKE_C_FLAGS_RELEASE "-O3 -fPIC -w")
    set(CMAKE_C_FLAGS_DEBUG "-Og -fPIC -w")
elseif(APPLE)
    set(CMAKE_C_COMPILER gcc)
    set(CMAKE_CXX_COMPILER g++)
    set(CMAKE_C_FLAGS_RELEASE "-O3 -fPIC -w -fembed-bitcode")
    set(CMAKE_C_FLAGS_DEBUG "-Og -fPIC -w")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_RELEASE}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_DEBUG}")
endif()

message(STATUS "CMAKE_CONFIGURATION_TYPES:= ${CMAKE_CONFIGURATION_TYPES}")
message(STATUS "CMAKE_C_FLAGS_RELEASE:= ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "CMAKE_C_FLAGS_DEBUG:= ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "CMAKE_C_FLAGS:= ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_BUILD_TYPE:= ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_INSTALL_PREFIX:= ${CMAKE_INSTALL_PREFIX}")

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/zbar/decoder")
include_directories("${PROJECT_SOURCE_DIR}/zbar/qrcode")
include_directories("${PROJECT_SOURCE_DIR}/zbar")
set(ZBAR_MAIN   "${PROJECT_SOURCE_DIR}/include/zbar.h")

set(LIOCONV     "lioconv-win")
set(LCONFIG     "lconfig")
set(LDECODER    "ldecoder")
set(LQRCODE     "lqrcode")
set(LZBAR       "")

if(WIN32)
    set(LZBAR       "libzbar")
elseif(ANDROID)
    message(STATUS "ANDROID_ABI = ${ANDROID_ABI}")
    set(LZBAR       "zbar")
elseif(IOS)
    message(STATUS "IOS_ARCH = ${IOS_ARCH}")
    set(LZBAR       "zbar")
else()    
    set(LZBAR       "zbar")
endif()

if(IOS)
    if(BUILD_SHARED_LIBS)
        set(COMPAT_VERSION 
            "-lSystem -compatibility_version ${PROJECT_VERSION} -current_version ${PROJECT_VERSION}"
            )
        set(CMAKE_MODULE_LINKER_FLAGS
            "-install_name @executable_path/Frameworks/zbar.framework/zbar ${COMPAT_VERSION}"
            )
        set(CMAKE_SHARED_LINKER_FLAGS
            "-install_name @executable_path/Frameworks/zbar.framework/zbar ${COMPAT_VERSION}"
            )
        set (CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG 1)
        set (CMAKE_INSTALL_NAME_DIR "@executable_path")
    endif()
endif()

if(WIN32 OR ANDROID OR APPLE OR IOS)
    include_directories("${PROJECT_SOURCE_DIR}/zbar/libiconv-win/include")
endif()

add_subdirectory("zbar")
set(PUBLIC_HEADERS
    "${PROJECT_SOURCE_DIR}/include/zbar.h"
)

set(NON_PUBLIC_HEADERS
    "${PROJECT_SOURCE_DIR}/include/zbar/zbargtk.h"
    "${PROJECT_SOURCE_DIR}/include/zbar/Symbol.h"
    "${PROJECT_SOURCE_DIR}/include/zbar/Exception.h"
    "${PROJECT_SOURCE_DIR}/include/zbar/Processor.h"
    "${PROJECT_SOURCE_DIR}/include/zbar/QZBar.h"
    "${PROJECT_SOURCE_DIR}/include/zbar/ImageScanner.h"
    "${PROJECT_SOURCE_DIR}/include/zbar/Decoder.h"
    "${PROJECT_SOURCE_DIR}/include/zbar/QZBarImage.h"
    "${PROJECT_SOURCE_DIR}/include/zbar/Video.h"
    "${PROJECT_SOURCE_DIR}/include/zbar/Scanner.h"
    "${PROJECT_SOURCE_DIR}/include/zbar/Image.h"
    "${PROJECT_SOURCE_DIR}/include/zbar/Window.h"
)

install(FILES ${PUBLIC_HEADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
install(FILES ${NON_PUBLIC_HEADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/zbar")
