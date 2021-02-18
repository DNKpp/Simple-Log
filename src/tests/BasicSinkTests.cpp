//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch2/catch.hpp"

#include "Simple-Log/BasicSink.hpp"
#include "Simple-Log/Record.hpp"

#include <algorithm>
#include <ranges>
#include <sstream>
#include <string_view>

using namespace sl::log;

using Record_t = BaseRecord<int, int>;
using BasicSink_t = BasicSink<Record_t>;

static std::size_t logAndCount(BasicSink_t& sink, std::ostringstream& out, std::string_view message)
{
	Record_t record;
	record.setMessage(std::string{ message });
	sink.log(record);

	auto outStr = out.str();

	auto itr = std::begin(outStr);
	std::size_t count = 0;
	for (;;)
	{
		itr = std::search(
						itr,
						std::end(outStr),
						std::boyer_moore_searcher{
							std::begin(message),
							std::end(message)
						}
						);
		if (itr == std::end(outStr))
		{
			break;
		}

		std::advance(itr, std::size(message));
		++count;
	}
	return count;
}

SCENARIO("log", "[BasicSink]")
{
	std::ostringstream out;
	BasicSink_t sink{ out };
	sink.enable();

	REQUIRE(std::empty(out.str()));

	WHEN("logging Records")
	{
		THEN("target ostream contains message string")
		{
			const std::string str = "Hello, World!";
			REQUIRE(logAndCount(sink, out, str) == 1);
		}
	}

	WHEN("logging multiple Records")
	{
		THEN("target ostream contains every message string")
		{
			const std::string str = "Hello, World!";
			for (std::size_t i = 1; i < 10; ++i)
			{
				REQUIRE(logAndCount(sink, out, str) == i);
			}
		}
	}
}

SCENARIO("enable", "[BasicSink]")
{
	std::ostringstream out;
	BasicSink_t sink{ out };

	REQUIRE(std::empty(out.str()));
	REQUIRE_FALSE(sink.isEnabled());

	WHEN("disabled")
	{
		AND_WHEN("enabling")
		{
			sink.enable();

			THEN("Sink becomes enabled")
			{
				REQUIRE(sink.isEnabled());
			}

			AND_WHEN("disabling again")
			{
				sink.enable(false);

				THEN("Sink becomes disabled again")
				{
					REQUIRE_FALSE(sink.isEnabled());
				}
			}

			AND_WHEN("enabling again")
			{
				THEN("Sink state doesn't change")
				{
					REQUIRE(sink.isEnabled());
				}
			}
		}

		AND_WHEN("logging Records")
		{
			THEN("Record will be skipped")
			{
				const std::string str = "Hello, World!";
				REQUIRE(logAndCount(sink, out, str) == 0);
			}
		}
	}

	WHEN("enabled")
	{
		sink.enable();

		AND_WHEN("logging Records")
		{
			THEN("target ostream contains message string")
			{
				const std::string str = "Hello, World!";
				REQUIRE(logAndCount(sink, out, str) == 1);
			}
		}
	}
}
