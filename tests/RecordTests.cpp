//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <chrono>
#include <string>
#include <type_traits>

#include "Simple-Log/Record.hpp"

using namespace sl::log;

using DefaultTimePoint_t = std::chrono::system_clock::time_point;

TEMPLATE_PRODUCT_TEST_CASE(
							"BaseRecords should have a valid state after construction succeeded",
							"[BaseRecord][Record]",
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
	REQUIRE(record.message() == RecordMessage_t<Record_t>{});
	REQUIRE(record.severity() == RecordSeverity_t<Record_t>{});
	REQUIRE(record.channel() == RecordChannel_t<Record_t>{});
	REQUIRE(record.timePoint() == RecordTimePoint_t<Record_t>{});
}

TEMPLATE_PRODUCT_TEST_CASE(
							"Record Getter abstraction should yield equal results as actual getters of BaseRecord type.",
							"[BaseRecord][Record]",
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
	using Getters_t = RecordGetters<Record_t>;

	Record_t record;
	REQUIRE(Getters_t::message(record) == RecordMessage_t<Record_t>{});
	REQUIRE(Getters_t::severity(record) == RecordSeverity_t<Record_t>{});
	REQUIRE(Getters_t::channel(record) == RecordChannel_t<Record_t>{});
	REQUIRE(Getters_t::timePoint(record) == RecordTimePoint_t<Record_t>{});
}

TEMPLATE_TEST_CASE_SIG(
						"BaseRecords getter should yield equal results after setter were used.",
						"[BaseRecord][Record]",
						((auto VSev, auto VChan, auto VMsg, auto VTimePoint), VSev, VChan, VMsg, VTimePoint),
						(1, 2, 3, 4)
					)
{
	using Record_t = BaseRecord<decltype(VSev), decltype(VChan), decltype(VMsg), decltype(VTimePoint)>;
	Record_t record;
	record.setSeverity(VSev);
	record.setChannel(VChan);
	record.setMessage(VMsg);
	record.setTimePoint(VTimePoint);

	REQUIRE(record.severity() == VSev);
	REQUIRE(record.channel() == VChan);
	REQUIRE(record.message() == VMsg);
	REQUIRE(record.timePoint() == VTimePoint);
}

TEMPLATE_TEST_CASE_SIG(
						"Record Setter abstraction should yield equal results as actual setters of BaseRecord type.",
						"[BaseRecord][Record]",
						((auto VSev, auto VChan, auto VMsg, auto VTimePoint), VSev, VChan, VMsg, VTimePoint),
						(1, 2, 3, 4)
					)
{
	using Record_t = BaseRecord<decltype(VSev), decltype(VChan), decltype(VMsg), decltype(VTimePoint)>;
	using Getters_t = RecordGetters<Record_t>;
	using Setters_t = RecordSetters<Record_t>;

	Record_t record;
	Setters_t::setMessage(record, VMsg);
	Setters_t::setSeverity(record, VSev);
	Setters_t::setChannel(record, VChan);
	Setters_t::setTimePoint(record, VTimePoint);

	REQUIRE(Getters_t::message(record) == VMsg);
	REQUIRE(Getters_t::severity(record) == VSev);
	REQUIRE(Getters_t::channel(record) == VChan);
	REQUIRE(Getters_t::timePoint(record) == VTimePoint);
}

namespace custom
{
	struct Record
	{
		std::string oMessage{};
		int oSev{};
		float oChan{};
		DefaultTimePoint_t oTimePoint{};
	};


}

template <>
struct sl::log::RecordTypedefs<custom::Record>
{
	using Message_t = std::string;
	using Severity_t = int;
	using Channel_t = float;
	using TimePoint_t = DefaultTimePoint_t;
};

template <>
struct sl::log::RecordGetters<custom::Record>
{
	constexpr static auto message{ [](custom::Record& record) { return record.oMessage; } };
	constexpr static auto severity{ [](custom::Record& record) { return record.oSev; } };
	constexpr static auto channel{ [](custom::Record& record) { return record.oChan; } };
	constexpr static auto timePoint{ [](custom::Record& record) { return record.oTimePoint; } };
};

template <>
struct sl::log::RecordSetters<custom::Record>
{
	constexpr static auto setMessage{ [](custom::Record& record, auto msg) { record.oMessage = msg; } };
	constexpr static auto setSeverity{ [](custom::Record& record, auto sev) { record.oSev = sev; } };
	constexpr static auto setChannel{ [](custom::Record& record, auto chan) { record.oChan = chan; } };
	constexpr static auto setTimePoint{ [](custom::Record& record, auto tPoint) { record.oTimePoint = tPoint; } };
};

TEST_CASE("Custom Record type using the abstractions should compile successfully.", "[Record]")
{
	using Record_t = custom::Record;
	using Typedefs_t = RecordTypedefs<Record_t>;
	using Getters_t = RecordGetters<Record_t>;
	using Setters_t = RecordSetters<Record_t>;

	REQUIRE(std::is_same_v<typename Typedefs_t::Message_t, std::string>);
	REQUIRE(std::is_same_v<typename Typedefs_t::Severity_t, int>);
	REQUIRE(std::is_same_v<typename Typedefs_t::Channel_t, float>);
	REQUIRE(std::is_same_v<typename Typedefs_t::TimePoint_t, DefaultTimePoint_t>);

	REQUIRE(std::is_same_v<RecordMessage_t<Record_t>, std::string>);
	REQUIRE(std::is_same_v<RecordSeverity_t<Record_t>, int>);
	REQUIRE(std::is_same_v<RecordChannel_t<Record_t>, float>);
	REQUIRE(std::is_same_v<RecordTimePoint_t<Record_t>, DefaultTimePoint_t>);

	const RecordMessage_t<Record_t> msg = "Hello, World!";
	const RecordSeverity_t<Record_t> sev = 1337;
	const RecordChannel_t<Record_t> chan = 13.37f;
	const RecordTimePoint_t<Record_t> tPoint = DefaultTimePoint_t::clock::now();

	Record_t record;
	Setters_t::setMessage(record, msg);
	Setters_t::setSeverity(record, sev);
	Setters_t::setChannel(record, chan);
	Setters_t::setTimePoint(record, tPoint);

	REQUIRE(Getters_t::message(record) == msg);
	REQUIRE(Getters_t::severity(record) == sev);
	REQUIRE(Getters_t::channel(record) == chan);
	REQUIRE(Getters_t::timePoint(record) == tPoint);
}