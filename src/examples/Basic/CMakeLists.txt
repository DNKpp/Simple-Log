cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(simple_log_example_basic)

if (SIMPLE_LOG_BUILD_EXAMPLES)

	add_executable(
		${PROJECT_NAME}
		main.cpp
	)

	target_link_libraries(
		${PROJECT_NAME}
		PRIVATE
		simple_log
	)

endif()
