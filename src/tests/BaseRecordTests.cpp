//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch2/catch.hpp"

#include "Simple-Log/PresetTypes.hpp"

using namespace sl::log;

using DefaultTimePoint_t = std::chrono::system_clock::time_point;

SCENARIO("default construction", "[BaseRecord]")
{
	WHEN("using std::string as message type")
	{
		BaseRecord<int, int, std::string> record;
		
		THEN("is empty")
		{
			REQUIRE(std::empty(record.message()));
			REQUIRE(record.severity() == 0);
			REQUIRE(record.channel() == 0);
			REQUIRE(record.timePoint() == DefaultTimePoint_t{});
		}
	}

	WHEN("using int as message type")
	{
		BaseRecord<int, int, int> record;

		THEN("equals zero")
		{
			REQUIRE(record.message() == 0);
			REQUIRE(record.severity() == 0);
			REQUIRE(record.channel() == 0);
			REQUIRE(record.timePoint() == DefaultTimePoint_t{});
		}
	}

	WHEN("using std::string as severity type")
	{
		BaseRecord<std::string, int> record;
		
		THEN("is empty")
		{
			REQUIRE(std::empty(record.message()));
			REQUIRE(std::empty(record.severity()));
			REQUIRE(record.channel() == 0);
			REQUIRE(record.timePoint() == DefaultTimePoint_t{});
		}
	}

	WHEN("using int as severity type")
	{
		BaseRecord<int, int> record;

		THEN("equals zero")
		{
			REQUIRE(std::empty(record.message()));
			REQUIRE(record.severity() == 0);
			REQUIRE(record.channel() == 0);
			REQUIRE(record.timePoint() == DefaultTimePoint_t{});
		}
	}

	WHEN("using std::string as channel type")
	{
		BaseRecord<int, std::string> record;
		
		THEN("is empty")
		{
			REQUIRE(std::empty(record.message()));
			REQUIRE(record.severity() == 0);
			REQUIRE(std::empty(record.channel()));
			REQUIRE(record.timePoint() == DefaultTimePoint_t{});
		}
	}

	WHEN("using int as channel type")
	{
		BaseRecord<int, int> record;

		THEN("equals zero")
		{
			REQUIRE(std::empty(record.message()));
			REQUIRE(record.severity() == 0);
			REQUIRE(record.severity() == 0);
			REQUIRE(record.timePoint() == DefaultTimePoint_t{});
		}
	}

	WHEN("using int as TimePoint type")
	{
		BaseRecord<int, int, std::string, int> record;
		
		THEN("equals zero")
		{
			REQUIRE(std::empty(record.message()));
			REQUIRE(record.severity() == 0);
			REQUIRE(record.channel() == 0);
			REQUIRE(record.timePoint() == 0);
		}
	}

	WHEN("using std::chrono::system_clock::time_point as channel type")
	{
		BaseRecord<int, int, std::string, DefaultTimePoint_t> record;

		THEN("default constructed std::chrono::system_clock::time_point")
		{
			REQUIRE(std::empty(record.message()));
			REQUIRE(record.severity() == 0);
			REQUIRE(record.severity() == 0);
			REQUIRE(record.timePoint() == DefaultTimePoint_t{});
		}
	}
}

SCENARIO("message", "[BaseRecord]")
{
	WHEN("using std::string as message type")
	{
		BaseRecord<int, int, std::string> record;

		AND_WHEN("setting message")
		{
			const auto* str = "Hello, World!";
			record.setMessage(str);
			
			THEN("contains that value")
			{
				REQUIRE(record.message() == str);
			}

			AND_THEN("everything else untouched")
			{
				REQUIRE(record.severity() == 0);
				REQUIRE(record.channel() == 0);
				REQUIRE(record.timePoint() == DefaultTimePoint_t{});
			}
		}
	}

	WHEN("using int as message type")
	{
		BaseRecord<int, int, int> record;

		AND_WHEN("setting message")
		{
			const int value = 1337;
			record.setMessage(value);
			
			THEN("contains that value")
			{
				REQUIRE(record.message() == value);
			}

			AND_THEN("everything else untouched")
			{
				REQUIRE(record.severity() == 0);
				REQUIRE(record.channel() == 0);
				REQUIRE(record.timePoint() == DefaultTimePoint_t{});
			}
		}
	}
}

SCENARIO("severity", "[BaseRecord]")
{
	WHEN("using std::string as severity type")
	{
		BaseRecord<std::string, int> record;

		AND_WHEN("setting severity")
		{
			const auto* str = "Hello, World!";
			record.setSeverity(str);
			
			THEN("contains that value")
			{
				REQUIRE(record.severity() == str);
			}
			
			AND_THEN("everything else untouched")
			{
				REQUIRE(std::empty(record.message()));
				REQUIRE(record.channel() == 0);
				REQUIRE(record.timePoint() == DefaultTimePoint_t{});
			}
		}
	}

	WHEN("using int as severity type")
	{
		BaseRecord<int, int> record;

		AND_WHEN("setting severity")
		{
			const int value = 1337;
			record.setSeverity(value);
			
			THEN("contains that value")
			{
				REQUIRE(record.severity() == value);
			}
			
			AND_THEN("everything else untouched")
			{
				REQUIRE(std::empty(record.message()));
				REQUIRE(record.channel() == 0);
				REQUIRE(record.timePoint() == DefaultTimePoint_t{});
			}
		}
	}
}

SCENARIO("channel", "[BaseRecord]")
{
	WHEN("using std::string as channel type")
	{
		BaseRecord<int, std::string> record;

		AND_WHEN("setting channel")
		{
			const auto* str = "Hello, World!";
			record.setChannel(str);
			
			THEN("contains that value")
			{
				REQUIRE(record.channel() == str);
			}

			AND_THEN("everything else untouched")
			{
				REQUIRE(std::empty(record.message()));
				REQUIRE(record.severity() == 0);
				REQUIRE(record.timePoint() == DefaultTimePoint_t{});
			}
		}
	}

	WHEN("using int as channel type")
	{
		BaseRecord<int, int> record;

		AND_WHEN("setting channel")
		{
			const int value = 1337;
			record.setChannel(value);
			
			THEN("contains that value")
			{
				REQUIRE(record.channel() == value);
			}

			AND_THEN("everything else untouched")
			{
				REQUIRE(std::empty(record.message()));
				REQUIRE(record.severity() == 0);
				REQUIRE(record.timePoint() == DefaultTimePoint_t{});
			}
		}
	}
}

SCENARIO("timePoint", "[BaseRecord]")
{
	WHEN("using int as timePoint type")
	{
		BaseRecord<int, int, std::string, int> record;

		AND_WHEN("setting timePoint")
		{
			const int value = 1337;
			record.setTimePoint(value);
			
			THEN("contains that value")
			{
				REQUIRE(record.timePoint() == value);
			}

			AND_THEN("everything else untouched")
			{
				REQUIRE(std::empty(record.message()));
				REQUIRE(record.severity() == 0);
				REQUIRE(record.channel() == 0);
			}
		}
	}

	WHEN("using std::chrono::system_clock::time_point as timePoint type")
	{
		BaseRecord<int, int, std::string, DefaultTimePoint_t> record;

		AND_WHEN("setting timePoint")
		{
			const auto value = std::chrono::system_clock::now();
			record.setTimePoint(value);
			
			THEN("contains that value")
			{
				REQUIRE(record.timePoint() == value);
			}

			AND_THEN("everything else untouched")
			{
				REQUIRE(std::empty(record.message()));
				REQUIRE(record.severity() == 0);
				REQUIRE(record.channel() == 0);
			}
		}
	}
}
