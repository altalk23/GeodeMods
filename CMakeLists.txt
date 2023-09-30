cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_OSX_ARCHITECTURES "x86_64")

project(GeodeMods)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
if (NOT DEFINED ENV{GEODE_SDK})
	message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
	message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()
add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)


if (APPLE)
	add_subdirectory(FPSBypass)
	add_subdirectory(FullscreenFix)
endif()
add_subdirectory(FPSShower)
add_subdirectory(SmallAdditions)
add_subdirectory(DrawingUtilities)