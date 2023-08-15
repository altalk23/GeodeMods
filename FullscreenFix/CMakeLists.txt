project(FullscreenFix VERSION 1.0.0)

file(GLOB OBJC_SOURCES
	src/*.mm
)
set_source_files_properties(${OBJC_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

add_library(${PROJECT_NAME} SHARED ${OBJC_SOURCES})

target_link_libraries(${PROJECT_NAME} "-framework Cocoa" "-framework OpenGL")

setup_geode_mod(${PROJECT_NAME})
