project(SmallAdditions VERSION 2.2.4)

file(GLOB SOURCES
	src/*.cpp
	src/fixes/*.cpp
	src/fixes/*.mm
	src/modifications/*.cpp
	src/optimizations/*.cpp
)

file(GLOB OBJC_SOURCES
	src/fixes/*.mm
)
set_source_files_properties(${OBJC_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${OBJC_SOURCES})

target_link_libraries(${PROJECT_NAME} "-framework Cocoa" "-framework OpenGL")

target_include_directories(${PROJECT_NAME} PRIVATE 
	include
	include/Dandelion
	include/Dandelion/fixes
	include/Dandelion/modifications
	include/Dandelion/optimizations
)

setup_geode_mod(${PROJECT_NAME})
