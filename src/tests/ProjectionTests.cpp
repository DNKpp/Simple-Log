//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch2/catch.hpp"

#include "Simple-Log/Projections.hpp"
#include "Simple-Log/Record.hpp"

using namespace sl::log;

SCENARIO("test Record std::any projections", "[record-projections]")
{
	Record rec;
	rec.severity = 3.5;
	rec.channel = "test";
	rec.userData = 1;

	WHEN("casting severity type")
	{
		AND_WHEN("to correct type")
		{
			THEN("cast will be successfull")
			{
				REQUIRE(proj::severityCast<double>(rec) != nullptr);
			}
		}

		AND_WHEN("to incorrect type")
		{
			THEN("cast will be unsuccessfull")
			{
				REQUIRE_FALSE(proj::severityCast<float>(rec) != nullptr);
				REQUIRE_FALSE(proj::severityCast<int>(rec) != nullptr);
				REQUIRE_FALSE(proj::severityCast<const char*>(rec) != nullptr);
				REQUIRE_FALSE(proj::severityCast<char[]>(rec) != nullptr);
			}
		}
	}

	WHEN("casting channel type")
	{
		AND_WHEN("to correct type")
		{
			THEN("cast will be successfull")
			{
				REQUIRE(proj::channelCast<const char*>(rec) != nullptr);
			}
		}

		AND_WHEN("to incorrect type")
		{
			THEN("cast will be unsuccessfull")
			{
				REQUIRE_FALSE(proj::channelCast<char[]>(rec) != nullptr);
				REQUIRE_FALSE(proj::channelCast<int>(rec) != nullptr);
				REQUIRE_FALSE(proj::channelCast<float>(rec) != nullptr);
				REQUIRE_FALSE(proj::channelCast<double>(rec) != nullptr);
			}
		}
	}

	WHEN("casting user_data type")
	{
		AND_WHEN("to correct type")
		{
			THEN("cast will be successfull")
			{
				REQUIRE(proj::userDataCast<int>(rec) != nullptr);
			}
		}

		AND_WHEN("to incorrect type")
		{
			THEN("cast will be unsuccessfull")
			{
				REQUIRE_FALSE(proj::userDataCast<char[]>(rec) != nullptr);
				REQUIRE_FALSE(proj::userDataCast<const char*>(rec) != nullptr);
				REQUIRE_FALSE(proj::userDataCast<float>(rec) != nullptr);
				REQUIRE_FALSE(proj::userDataCast<double>(rec) != nullptr);
			}
		}
	}
}
