cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(simple_log_test)

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
	CoreTests.cpp
	BaseRecordTests.cpp
	TupleAlgorithmTests.cpp
	PredicateTests.cpp
	FilterTests.cpp
	RecordQueueTests.cpp
	StringPatternTest.cpp
	FlushPolicyTests.cpp
	OStreamSinkTests.cpp
	FileSinkTests.cpp
	ConsoleSinkTests.cpp
	LoggerTests.cpp
	RecordBuilderTests.cpp
	BasicSinkTests.cpp
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
	simple_log
	Catch2::Catch2
)

add_test(SimpleLogTestSuite ${PROJECT_NAME})
