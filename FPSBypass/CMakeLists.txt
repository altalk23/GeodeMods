project(FPSBypass VERSION 1.0.8)

file(GLOB OBJC_SOURCES
	src/*.mm
)
set_source_files_properties(${OBJC_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

add_library(${PROJECT_NAME} SHARED ${OBJC_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} "-framework Cocoa" "-framework OpenGL" "-framework CoreVideo")

setup_geode_mod(FPSBypass)
