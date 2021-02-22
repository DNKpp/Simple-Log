//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <chrono>
#include <string>

#include "Simple-Log/Record.hpp"

using namespace sl::log;

using DefaultTimePoint_t = std::chrono::system_clock::time_point;

TEMPLATE_PRODUCT_TEST_CASE(
							"BaseRecords should have a valid state after construction succeeded",
							"[BaseRecord]",
							BaseRecord,
							(
								(int, int, int, int),
								(std::string, int, int, int),
								(int, std::string, int, int),
								(int, int, std::string, int),
								(int, int, int, std::string)
							)
						)
{
	using Record_t = TestType;

	Record_t record;
	REQUIRE(record.message() == typename Record_t::Message_t{});
	REQUIRE(record.severity() == typename Record_t::SeverityLevel_t{});
	REQUIRE(record.channel() == typename Record_t::Channel_t{});
	REQUIRE(record.timePoint() == typename Record_t::TimePoint_t{});
}

TEMPLATE_TEST_CASE_SIG(
						"BaseRecords getter should yield equal results after setter were used.",
						"[BaseRecord]",
						((auto sev, auto chan, auto msg, auto timePoint), sev, chan, msg, timePoint),
						(1, 2, 3, 4)
					)
{
	using Record_t = BaseRecord<decltype(sev), decltype(chan), decltype(msg), decltype(timePoint)>;
	Record_t record;
	record.setSeverity(sev);
	record.setChannel(chan);
	record.setMessage(msg);
	record.setTimePoint(timePoint);

	REQUIRE(record.severity() == sev);
	REQUIRE(record.channel() == chan);
	REQUIRE(record.message() == msg);
	REQUIRE(record.timePoint() == timePoint);
}
