//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <filesystem>
#include <optional>
#include <string>

#include "Simple-Log/FileSink.hpp"
#include "Simple-Log/Record.hpp"

using namespace sl::log;
using namespace std::literals;

using Record_t = BaseRecord<int, int, int>;
using FileSink_t = FileSink<Record_t>;

inline std::filesystem::path defaultTestDir = std::filesystem::current_path() / "FileSinkTest";

void cleanupDefaultDirectory()
{
	remove_all(defaultTestDir);
}

SCENARIO("FileSink should have a valid state after construction succeeded", "[FileSink][Sink]")
{
	cleanupDefaultDirectory();

	const auto fileNamePattern = "log.file"s;

	GIVEN("an non-empty file name pattern string")
	WHEN("constructing a FileSink instance with that pattern string as argument")
	THEN("fileNamePattern should return an equal pattern string")
	{
		FileSink_t sink{ fileNamePattern };
		REQUIRE(sink.fileNamePattern() == fileNamePattern);
	}

	GIVEN("an arbitrary empty file name pattern string")
	WHEN("constructing a FileSink instance with that pattern string as argument")
	THEN("construction should throw an SinkException")
	{
		REQUIRE_THROWS_AS(FileSink_t{ "" }, SinkException);
	}

	GIVEN("an arbitrary directory")
	WHEN("constructing a FileSink instance with directory as argument")
	{
		FileSink_t sink{ fileNamePattern, defaultTestDir };
		THEN("directory should return that directory")
		{
			REQUIRE(sink.directory() == defaultTestDir);
		}

		THEN("directory should exist after construction")
		{
			REQUIRE(is_directory(sink.directory()));
		}
	}

	GIVEN("a FileSink instance with un-specified directory")
	WHEN("setting directory afterwards")
	THEN("directory should exist")
	{
		FileSink_t sink{ "log.file" };
		sink.setDirectory(defaultTestDir);
		REQUIRE(is_directory(sink.directory()));
	}
}

SCENARIO("FileSink::setFileNamePattern should modify member.", "[FileSink][Sink]")
{
	const auto fileNamePattern = "log.file"s;
	FileSink_t sink{ fileNamePattern + ".t", defaultTestDir };

	GIVEN("an arbitrary FileSink instance")
	{
		WHEN("setting file pattern name with an non-empty string")
		THEN("fileNamePattern should return an equal pattern string")
		{
			sink.setFileNamePattern(fileNamePattern);

			REQUIRE(sink.fileNamePattern() == fileNamePattern);
		}

		WHEN("setting file pattern name with an empty string")
		THEN("setFileNamePattern should throw an SinkException")
		{
			REQUIRE_THROWS_AS(sink.setFileNamePattern(""), SinkException);
		}
	}
}

SCENARIO("FileSink::setDirectory should modify member.", "[FileSink][Sink]")
{
	const auto fileNamePattern = "log.file"s;
	FileSink_t sink{ fileNamePattern };

	GIVEN("an arbitrary FileSink instance")
	{
		WHEN("setting directory")
		{
			sink.setDirectory(defaultTestDir);
			THEN("directory should return similar path")
			{
				REQUIRE(sink.directory() == defaultTestDir);
			}

			AND_THEN("directory should exist after FileSink construction")
			{
				REQUIRE(is_directory(sink.directory()));
			}
		}
	}
}

SCENARIO("FileSink's rotate function should switch files.", "[FileSink][Sink]")
{
	cleanupDefaultDirectory();

	const auto fileNamePattern = "log%N.file"s;
	FileSink_t sink{ fileNamePattern, defaultTestDir };

	GIVEN("a FileSink instance which doesn't have an open file yet")
	WHEN("calling rotate")
	THEN("nothing should happen")
	{
		REQUIRE_FALSE(exists(sink.directory() / "log1.file"));

		sink.rotate();
		REQUIRE_FALSE(exists(sink.directory() / "log1.file"));
	}

	GIVEN("a FileSink instance which has an existing and open file")
	WHEN("calling rotate")
	THEN("sink should switch to new file")
	{
		sink.setEnabled();
		sink.log({});

		REQUIRE(exists(sink.directory() / "log1.file"));
		sink.rotate();
		REQUIRE(exists(sink.directory() / "log2.file"));
	}
}

SCENARIO("FileSink openingHandler property should get invoked on file opening.", "[FileSink][Sink]")
{
	const auto fileNamePattern = "log.file"s;

	FileSink_t sink{ fileNamePattern, defaultTestDir };
	bool invoked = false;
	sink.setOpeningHandler(
							[&invoked]()
							{
								invoked = true;
								return "";
							}
						);

	GIVEN("a disabled FileSink instance")
	WHEN("opening a file by logging the first record")
	THEN("logging should not invoke the openingHandler")
	{
		sink.log({});
		REQUIRE_FALSE(invoked);
	}

	GIVEN("an enabled FileSink instance")
	{
		sink.setEnabled();
		sink.log({});
		WHEN("opening a file by logging the first record")
		THEN("openingHandler should get invoked")
		{
			REQUIRE(invoked);
		}

		WHEN("rotating a file by calling rotate method")
		THEN("openingHandler should get invoked")
		{
			sink.rotate();
			REQUIRE(invoked);
		}
	}
}

SCENARIO("FileSink closingHandler property should get invoked on file closing.", "[FileSink][Sink]")
{
	const auto fileNamePattern = "log.file"s;

	std::optional<FileSink_t> sink{ std::in_place, fileNamePattern, defaultTestDir };
	bool invoked = false;
	sink->setClosingHandler(
							[&invoked]()
							{
								invoked = true;
								return "";
							}
							);

	sink->setEnabled();
	sink->log({});

	GIVEN("an enabled FileSink instance which has an existing and open file")
	{
		WHEN("rotating a file by calling rotate method")
		THEN("closingHandler should get invoked")
		{
			sink->rotate();
			REQUIRE(invoked);
		}

		WHEN("destructing this FileSink instance")
		THEN("closingHandler should get invoked")
		{
			sink.reset();
			REQUIRE(invoked);
		}
	}
}
