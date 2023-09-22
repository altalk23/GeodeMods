project(DrawingUtilities VERSION 0.2.2)

file(GLOB SOURCES
	src/*.cpp
)

CPMAddPackage("gh:memononen/nanosvg#9da543e")

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    include
    ${nanosvg_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} nanosvg)

setup_geode_mod(${PROJECT_NAME})
