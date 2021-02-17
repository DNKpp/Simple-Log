cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(simple_log_example_basic)

add_executable(
	${PROJECT_NAME}
	main.cpp
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
	simple_log
)
