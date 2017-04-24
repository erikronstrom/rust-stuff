CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

SET( PROJ_NAME      "gaudiamus" )
SET( PROJ_PATH      ${CMAKE_SOURCE_DIR} )
SET( PROJ_OUT_PATH  ${CMAKE_BINARY_DIR} )
FILE( GLOB_RECURSE PROJ_SOURCES src/*.c ) # src/ga/*.c )
FILE( GLOB_RECURSE PROJ_HEADERS include/*.h )
SET( PROJ_INCLUDES  "include" )

PROJECT( ${PROJ_NAME} )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")




## ----------------------------------------------------------------------
## Basic definitions

project(FA C CXX)
set(VERSION_PRE "")
set(VERSION_X 2)
set(VERSION_Y 22)
set(VERSION_Z 0)
set(VERSION_SUFF "")
set(VERSION "${VERSION_PRE}${VERSION_X}.${VERSION_Y}.${VERSION_Z}${VERSION_SUFF}")




if(APPLE)
  set(NOT_APPLE False)
else()
  set(NOT_APPLE True)
endif()
if(WIN32) # TODO better check
  set(MSYS     True)
  set(NOT_MSYS False)
else()
  set(MSYS     False)
  set(NOT_MSYS True)
endif()

if (APPLE)
  set(CMAKE_C_COMPILER "/usr/bin/clang")
  add_definitions(
      -Wall
      )
  add_definitions("-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
  add_definitions("-isysroot ${CMAKE_OSX_SYSROOT}")
  # add_definitions("-v")
  
  set(STD_LINK_FLAGS "${STD_LINK_FLAGS} -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
  set(STD_LINK_FLAGS "${STD_LINK_FLAGS} -isysroot ${CMAKE_OSX_SYSROOT}")
  set(STD_LINK_FLAGS "${STD_LINK_FLAGS} -ObjC -all_load")
endif()

if (MSYS)
  add_definitions(-std=c11)
endif()

set(CMAKE_C_FLAGS_DEBUG "-DGA_DEBUG")

include_directories("include")



## ----------------------------------------------------------------------
## Build options


option(ENABLE_FLUIDSYNTH        "Enable the Fluidsynth processor"           ${NOT_APPLE})
option(ENABLE_OGG_VORBIS        "Enable OGG/Vorbis support"                 True)
option(ENABLE_AUDIO_UNIT        "Enable AudioUnit processors"               ${APPLE})
option(ENABLE_VST               "Enable VST processors"                     True)
option(ENABLE_MP3_IMPORT        "Enable MP3 import"                         True)


## ----------------------------------------------------------------------
## Sanity checks


if(APPLE)
  # string(COMPARE NOTEQUAL "${CMAKE_OSX_ARCHITECTURES}" "i386" ARCH_NOT_32_BIT)
  # if(ARCH_NOT_32_BIT)
  #   message(FATAL_ERROR
  #     "\nThe Audio Engine does not support 64-bit mode yet."
  #     "\nTry -DCMAKE_OSX_ARCHITECTURES=i386.")
  # endif()

  string(COMPARE EQUAL "${CMAKE_OSX_DEPLOYMENT_TARGET}" "" NO_DEP_TARGET)
  if(NO_DEP_TARGET)
    message(FATAL_ERROR
      "\nNo OS X deployment target set."
      "\nTry -DCMAKE_OSX_DEPLOYMENT_TARGET=10.7.")
  endif()
endif()

if(MSYS)
  string(COMPARE NOTEQUAL "${CMAKE_GENERATOR}" "MSYS Makefiles" GENERATOR_NOT_MSYS)
  if(GENERATOR_NOT_MSYS)
    message(FATAL_ERROR
      "\nMust use MSYS Makefiles generator."
      "\nTry -G 'MSYS Makefiles'.")
  endif()
endif()


## ----------------------------------------------------------------------
## Add libraries


# liblo
find_package(liblo REQUIRED)
include_directories(${LIBLO_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LIBLO_LIBRARIES})

# Fluidsynth
if(ENABLE_FLUIDSYNTH)
  find_package(Fluidsynth)
  find_package(Glib)
  include_directories(${FLUIDSYNTH_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${FLUIDSYNTH_LIBRARIES})
endif()


## ----------------------------------------------------------------------
## Check c functions

include(CheckFunctionExists)
check_function_exists(strdup HAVE_STRDUP)


## ----------------------------------------------------------------------
## Config file


set(GA_FLUIDSYNTH         ${ENABLE_FLUIDSYNTH})
set(GA_OGG_VORBIS         ${ENABLE_OGG_VORBIS})
set(GA_AUDIO_UNIT         ${ENABLE_AUDIO_UNIT})
set(GA_VST                ${ENABLE_VST})
set(GA_MP3_IMPORT         ${ENABLE_MP3_IMPORT})
set(MACOSX                ${APPLE})
set(WINDOWS               ${WIN32})

configure_file(
  ${CMAKE_SOURCE_DIR}/include/config.h.in
  ${CMAKE_BINARY_DIR}/include/config.h
 )


## ----------------------------------------------------------------------
## Set target(s)

include_directories( ${PROJ_INCLUDES} ${CMAKE_BINARY_DIR}/include )
add_executable( ${PROJ_NAME} ${PROJ_SOURCES} )
target_link_libraries( ${PROJ_NAME}
  ${LIBS}
  )
