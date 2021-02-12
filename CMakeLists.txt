cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(simple_log)

option(SIMPLE_LOG_BUILD_EXAMPLES "Enables example building" OFF)

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

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	include(CTest)
	include(FetchContent)

	FetchContent_Declare(
	  catch2
	  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	  GIT_TAG        origin/v2.x
	)
	FetchContent_MakeAvailable(catch2)

	add_executable(
		test_simple_log
		${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/test/StringPatternTest.cpp
		#${CMAKE_CURRENT_SOURCE_DIR}/test/LogTest.cpp
	)

	target_link_libraries(
		test_simple_log
		PRIVATE
		simple_log
		Catch2::Catch2
	)

	enable_testing()
	add_test(
		NAME SimpleLogTestSuite
		COMMAND test_simple_log
	)
endif()

if (SIMPLE_LOG_BUILD_EXAMPLES)

	add_executable(
		example_simple_log_basic
		${ExamplesDir}Basic/main.cpp
	)

	target_link_libraries(
		example_simple_log_basic
		PRIVATE
		simple_log
	)

endif()
