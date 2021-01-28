cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(simple_log)

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

	add_executable(
		test_simple_log
		${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/test/LogTest.cpp
	)

	target_link_libraries(
		test_simple_log
		PRIVATE
		simple_log
	)

	enable_testing()
	add_test(
		NAME SimpleLogTestSuite
		COMMAND test_simple_log
	)
endif()