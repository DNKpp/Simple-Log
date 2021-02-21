//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <filesystem>
#include <string>

#include "Simple-Log/FileSink.hpp"
#include "Simple-Log/Record.hpp"

using namespace sl::log;
using namespace std::literals;

using Record_t = BaseRecord<int, int, int>;
using FileSink_t = FileSink<Record_t>;

inline std::filesystem::path defaultDir = std::filesystem::current_path() / "FileSinkTest";

void cleanupDefaultDirectory()
{
	remove_all(defaultDir);
}

SCENARIO("FileSink should use a valid file name pattern", "[FileSink][Sink]")
{
	const auto fileNamePattern = "log.file"s;
	GIVEN("a FileSink instance with specified file name pattern")
	{
		FileSink_t sink{ fileNamePattern + ".t" };
		WHEN("construction is finished")
		THEN("fileNamePattern should return an equal pattern string")
		{
			REQUIRE(sink.fileNamePattern() == fileNamePattern + ".t");
		}

		AND_WHEN("setting fileNamePattern afterwards")
		{
			THEN("fileNamePattern should again return an equal pattern string")
			{
				sink.setFileNamePattern(fileNamePattern);
				REQUIRE(sink.fileNamePattern() == fileNamePattern);
			}
		}

		AND_WHEN("setting an empty fileNamePattern")
		{
			THEN("setFileNamePattern should throw")
			{
				REQUIRE_THROWS_AS(sink.setFileNamePattern(""), SinkException);
			}
		}
	}
}

SCENARIO("FileSink should use a reachable directory", "[FileSink][Sink]")
{
	cleanupDefaultDirectory();

	GIVEN("a FileSink instance with specified directory")
	WHEN("construction is finished")
	{
		FileSink_t sink{ "log.file", defaultDir };
		THEN("directory should return that directory")
		{
			REQUIRE(sink.directory() == defaultDir);
		}

		AND_THEN("directory should exist after FileSink construction")
		{
			REQUIRE(is_directory(sink.directory()));
		}
	}

	AND_GIVEN("a FileSink instance with un-specified directory")
	WHEN("setting directory afterwards")
	{
		FileSink_t sink{ "log.file" };
		THEN("directory should exist")
		{
			sink.setDirectory(defaultDir);
			REQUIRE(is_directory(sink.directory()));
		}
	}
}

//TEST_CASE("", "[FileSink][Sink]")
//{
//	const std::string_view fileName = "test.log";
//	auto dir = std::filesystem::current_path();
//	WHEN("FileSink points to default directory")
//	{
//	}
//
//	WHEN("FileSink points to sub-directory")
//	{
//		dir /= "test_sub";
//	}
//
//	FileSink<Record_t> sink{ std::string(fileName), dir };
//	sink.enable();
//
//	auto filePath = dir / fileName;
//	auto begin = std::filesystem::file_time_type::clock::now();
//
//	const Record_t record;
//	sink.log(record);
//
//	// seems to fail on last_write_time if not slept before, but that doesn't matter as I'm only
//	// interested in if the file is created and some data is written.
//	std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
//
//	REQUIRE(is_regular_file(filePath));
//}
