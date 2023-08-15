project(FPSShower VERSION 1.0.1)

file(GLOB SOURCES
	src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    include/
)

setup_geode_mod(${PROJECT_NAME})
