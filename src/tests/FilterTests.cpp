//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <functional>

#include "Simple-Log/Filters.hpp"
#include "Simple-Log/Predicates.hpp"
#include "Simple-Log/Record.hpp"

using namespace sl::log;

using Record_t = BaseRecord<int, int, int, int>;

TEST_CASE("ProjectionFilter should watch on specified Record member", "[Filter]")
{
	ProjectionFilter filter{ &Record_t::message, Equals{ 0 } };

	SECTION("Approve Records with positive predicate result")
	{
		Record_t record;
		record.setMessage(0);

		REQUIRE(filter(record));
	}

	SECTION("Reject Records with negative predicate result")
	{
		Record_t record;
		record.setMessage(1);

		REQUIRE_FALSE(filter(record));
	}
}

TEST_CASE("makeMessageFilterFor function should create Filter watching on Record's message member", "[Filter]")
{
	Record_t record;
	auto filter = makeMessageFilterFor<Record_t>(Equals{ 1337 });

	REQUIRE_FALSE(std::invoke(filter, record));

	record.setMessage(1337);

	REQUIRE(std::invoke(filter, record));
}

TEST_CASE("makeSeverityFilterFor function should create Filter watching on Record's severity member", "[Filter]")
{
	Record_t record;
	auto filter = makeSeverityFilterFor<Record_t>(Equals{ 1337 });

	REQUIRE_FALSE(std::invoke(filter, record));

	record.setSeverity(1337);

	REQUIRE(std::invoke(filter, record));
}

TEST_CASE("makeChannelFilterFor function should create Filter watching on Record's channel member", "[Filter]")
{
	Record_t record;
	auto filter = makeChannelFilterFor<Record_t>(Equals{ 1337 });

	REQUIRE_FALSE(std::invoke(filter, record));

	record.setChannel(1337);

	REQUIRE(std::invoke(filter, record));
}

TEST_CASE("makeTimePointFilterFor function should create Filter watching on Record's timePoint member", "[Filter]")
{
	Record_t record;
	auto filter = makeTimePointFilterFor<Record_t>(Equals{ 1337 });

	REQUIRE_FALSE(std::invoke(filter, record));

	record.setTimePoint(1337);

	REQUIRE(std::invoke(filter, record));
}

// ToDo: Enable if clang is able to compile
//TEMPLATE_TEST_CASE_SIG(
//						"make...FilterFor functions should create Filters watching for the corresponding Record member",
//						"[Filter]",
//						((auto MakeFunction, auto SetterFunc), MakeFunction, SetterFunc),
//						([](auto predicate) { return makeMessageFilterFor<Record_t>(predicate); },
//							[](Record_t& rec, auto msg) { rec.setMessage(std::move(msg)); }),
//						([](auto predicate) { return makeSeverityFilterFor<Record_t>(predicate); },
//							[](Record_t& rec, auto msg) { rec.setSeverity(std::move(msg)); }),
//						([](auto predicate) { return makeChannelFilterFor<Record_t>(predicate); },
//							[](Record_t& rec, auto msg) { rec.setChannel(std::move(msg)); }),
//						([](auto predicate) { return makeTimePointFilterFor<Record_t>(predicate); },
//							[](Record_t& rec, auto msg) { rec.setTimePoint(std::move(msg)); })
//					)
//{
//	Record_t record;
//	auto filter = std::invoke(MakeFunction, Equals{ 1337 });
//
//	REQUIRE_FALSE(std::invoke(filter, record));
//
//	std::invoke(SetterFunc, record, 1337);
//
//	REQUIRE(std::invoke(filter, record));
//}

TEST_CASE("FilterChain should determine the amount of attached sub-filters", "[Filter]")
{
	SECTION("With no attached sub-filters")
	{
		FilterChain filterChain
		{
			detail::TupleAllOf{},
		};

		REQUIRE(filterChain.empty());
		REQUIRE(filterChain.size() == 0);
	}

	SECTION("With some attached sub-filters")
	{
		constexpr auto dummyFilter = [](const auto& rec)
		{
			return true;
		};

		FilterChain filterChain
		{
			detail::TupleAllOf{},
			dummyFilter,
			dummyFilter
		};

		REQUIRE_FALSE(filterChain.empty());
		REQUIRE(filterChain.size() == 2);
	}
}

TEST_CASE("FilterChain should determine invocation result based on their applied algorithm", "[Filter]")
{
	SECTION("With AllOf algorithm, and each sub-filter returning true, every filter should be invoked")
	{
		int invokeCounter = 0;
		auto countedPred = [&invokeCounter](const auto& rec)
		{
			++invokeCounter;
			return true;
		};

		FilterChain filter
		{
			detail::TupleAllOf{},
			countedPred,
			countedPred,
			countedPred
		};

		filter(Record_t{});

		REQUIRE(invokeCounter == std::size(filter));
	}
}
