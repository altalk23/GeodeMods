project(DrawingUtilities VERSION 0.2.0)

file(GLOB SOURCES
	src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    include
)

setup_geode_mod(${PROJECT_NAME})
