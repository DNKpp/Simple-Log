cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(simple_log
	VERSION 0.6
	LANGUAGES CXX
)

include(CTest)

option(SIMPLE_LOG_BUILD_EXAMPLES "Enables example building" OFF)

set(TestsDir "${CMAKE_CURRENT_SOURCE_DIR}/src/tests/")
set(ExamplesDir "${CMAKE_CURRENT_SOURCE_DIR}/src/examples/")

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
	${PROJECT_NAME}
	INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(
	${PROJECT_NAME}
	INTERFACE
	cxx_std_20
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_link_options(${PROJECT_NAME}
		INTERFACE -pthread -latomic
	)
endif()

if (BUILD_TESTING)
	add_subdirectory(${TestsDir})
endif()

if (SIMPLE_LOG_BUILD_EXAMPLES)
	add_subdirectory(${ExamplesDir})
endif()
