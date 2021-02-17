cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(simple_log_test)

option(SIMPLE_LOG_BUILD_TESTS "Enables test building" OFF)

if(SIMPLE_LOG_BUILD_TESTS)
	include(CTest)
	include(FetchContent)

	FetchContent_Declare(
	  catch2
	  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	  GIT_TAG        origin/v2.x
	)
	FetchContent_MakeAvailable(catch2)

	add_executable(
		${PROJECT_NAME}
		main.cpp
		TupleAlgorithmTests.cpp
		PredicateTests.cpp
		FilterTests.cpp
		
		StringPatternTest.cpp
		#LogTest.cpp
	)

	target_link_libraries(
		${PROJECT_NAME}
		PRIVATE
		simple_log
		Catch2::Catch2
	)

	enable_testing()
	add_test(
		NAME SimpleLogTestSuite
		COMMAND ${PROJECT_NAME}
	)
endif()
