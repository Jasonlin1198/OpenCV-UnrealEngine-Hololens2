# Install script for directory: C:/Users/jason/Downloads/libzbar-master/libzbar-master

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/lib/libzbar")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/lib/libzbar/include/zbar.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "C:/lib/libzbar/include" TYPE FILE FILES "C:/Users/jason/Downloads/libzbar-master/libzbar-master/include/zbar.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/lib/libzbar/include/zbar/zbargtk.h;C:/lib/libzbar/include/zbar/Symbol.h;C:/lib/libzbar/include/zbar/Exception.h;C:/lib/libzbar/include/zbar/Processor.h;C:/lib/libzbar/include/zbar/QZBar.h;C:/lib/libzbar/include/zbar/ImageScanner.h;C:/lib/libzbar/include/zbar/Decoder.h;C:/lib/libzbar/include/zbar/QZBarImage.h;C:/lib/libzbar/include/zbar/Video.h;C:/lib/libzbar/include/zbar/Scanner.h;C:/lib/libzbar/include/zbar/Image.h;C:/lib/libzbar/include/zbar/Window.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "C:/lib/libzbar/include/zbar" TYPE FILE FILES
    "C:/Users/jason/Downloads/libzbar-master/libzbar-master/include/zbar/zbargtk.h"
    "C:/Users/jason/Downloads/libzbar-master/libzbar-master/include/zbar/Symbol.h"
    "C:/Users/jason/Downloads/libzbar-master/libzbar-master/include/zbar/Exception.h"
    "C:/Users/jason/Downloads/libzbar-master/libzbar-master/include/zbar/Processor.h"
    "C:/Users/jason/Downloads/libzbar-master/libzbar-master/include/zbar/QZBar.h"
    "C:/Users/jason/Downloads/libzbar-master/libzbar-master/include/zbar/ImageScanner.h"
    "C:/Users/jason/Downloads/libzbar-master/libzbar-master/include/zbar/Decoder.h"
    "C:/Users/jason/Downloads/libzbar-master/libzbar-master/include/zbar/QZBarImage.h"
    "C:/Users/jason/Downloads/libzbar-master/libzbar-master/include/zbar/Video.h"
    "C:/Users/jason/Downloads/libzbar-master/libzbar-master/include/zbar/Scanner.h"
    "C:/Users/jason/Downloads/libzbar-master/libzbar-master/include/zbar/Image.h"
    "C:/Users/jason/Downloads/libzbar-master/libzbar-master/include/zbar/Window.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/jason/Downloads/libzbar-master/libzbar-master/zbar/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/jason/Downloads/libzbar-master/libzbar-master/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
