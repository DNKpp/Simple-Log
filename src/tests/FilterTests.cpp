//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch2/catch.hpp"

#include "Simple-Log/Filters.hpp"
#include "Simple-Log/PresetTypes.hpp"

#include <string>

using namespace sl::log;

SCENARIO("test ProjectionFilter", "[filters]")
{
	pre::Record_t record;
	constexpr auto str = "Hello, World!";
	record.setMessage(str);
	
	WHEN("hand-over message projection and querying for that string")
	{
		ProjectionFilter filter{ &pre::Record_t::message, EqualsTo{ str }};
		THEN("returns true")
		{
			REQUIRE(filter(record));
		}
	}

	WHEN("hand-over message projection and querying for another string")
	{
		ProjectionFilter filter{ &pre::Record_t::message, EqualsTo{ str + std::string("_not") }};
		THEN("returns false")
		{
			REQUIRE_FALSE(filter(record));
		}
	}
}

SCENARIO("test FilterChain", "[filters]")
{
	pre::Record_t record;
	constexpr auto str = "Hello, World!";
	record.setMessage(str);
	
	WHEN("filling with constant true returns and counting calls")
	{
		int callCounter = 0;
		auto countedPred = [&callCounter](const auto& rec) { ++callCounter; return true; };
		FilterChain filters
		{
			detail::TupleAllOf{},
			countedPred,
			countedPred,
			countedPred
		};
		THEN("returns true and requires correct call counting")
		{
			REQUIRE(filters(record));
			REQUIRE(callCounter == 3);
		}
	}
}
