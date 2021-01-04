cmake_minimum_required(VERSION 3.13)

project(enqu LANGUAGES C CXX)

list(APPEND CMAKE_LIBRARY_PATH ${LIBRARY_PATH} $ENV{LIBRARY_PATH})
list(APPEND CMAKE_INCLUDE_PATH ${INCLUDE_PATH} $ENV{INCLUDE_PATH})

set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX)
 link_libraries(pthread dl)
else()
 add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
 add_compile_options("-ffunction-sections")
 add_link_options("-Wl,--gc-sections")
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
add_definitions(-DQT_NO_KEYWORDS)

find_library(VSS_LIB NAMES vapoursynth-script vsscript)
find_library(X265_LIB NAMES x265 libx265)

find_path(VAPOURSYNTH_DIR NAMES vapoursynth.h PATH_SUFFIXES vapoursynth)
find_path(X265_DIR NAMES x265.h)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${VAPOURSYNTH_DIR} ${X265_DIR})

add_executable(enqu enqu.cxx enqu_x265.cxx main.cxx enqu.h enqu_x265.h main.h)

set_target_properties(enqu PROPERTIES CXX_STANDARD 20 VISIBILITY_INLINES_HIDDEN 1 CXX_VISIBILITY_PRESET hidden C_VISIBILITY_PRESET hidden)

target_link_libraries(enqu Qt5::Widgets ${VSS_LIB} ${X265_LIB})
