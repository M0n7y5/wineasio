cmake_minimum_required(VERSION 3.20)
project(pipeasio C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
                 Debug Release RelWithDebInfo)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# libpipewire-0.3 backs src/audio.c — its headers feed the OBJECT lib
# compile step via INCLUDES, and -lpipewire-0.3 reaches the winegcc link
# step via LIBS.
find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED IMPORTED_TARGET libpipewire-0.3)
message(STATUS "Found libpipewire-0.3 ${PIPEWIRE_VERSION}")

include(WineDLL)

add_wine_dll(
    NAME      pipeasio64
    SPEC      ${CMAKE_SOURCE_DIR}/pipeasio.dll.spec
    SOURCES   src/asio.c
              src/audio.c
              src/main.c
              src/regsvr.c
    INCLUDES  ${CMAKE_SOURCE_DIR}/include
              ${PIPEWIRE_INCLUDE_DIRS}
    LIBS      odbc32 ole32 uuid winmm pipewire-0.3
)
