//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include "Simple-Log/ConsoleSink.hpp"
#include "Simple-Log/PresetTypes.hpp"
#include "Simple-Log/Record.hpp"

using namespace sl::log;
using namespace std::literals;

using Record_t = preset::Record_t;
using ConsoleSink_t = ConsoleSink<Record_t>;

TEST_CASE("ConsoleTextStyleTable should yield the same styles as inserted during construction.", "[ConsoleSink][Sink]")
{
	using Style = ConsoleTextStyle;
	using SevLvl = preset::SevLvl;
	auto styleTable = makeConsoleTextStyleTableFor<Record_t>(
															&Record_t::severity,
															{
																{ SevLvl::info, Style{ .bgColor = Style::BgColor::blue } },
																{ SevLvl::debug, Style{ .bgColor = Style::BgColor::yellow } },
																{ SevLvl::error, Style{ .bgColor = Style::BgColor::red } }
															}
															);

	Record_t record;
	record.setSeverity(SevLvl::info);
	REQUIRE(std::get<Style::BgColor>(styleTable(record).bgColor) == Style::BgColor::blue);
	record.setSeverity(SevLvl::debug);
	REQUIRE(std::get<Style::BgColor>(styleTable(record).bgColor) == Style::BgColor::yellow);
	record.setSeverity(SevLvl::error);
	REQUIRE(std::get<Style::BgColor>(styleTable(record).bgColor) == Style::BgColor::red);
	record.setSeverity(SevLvl::warning);
	REQUIRE(std::get<Style::BgColor>(styleTable(record).bgColor) == Style::BgColor::reset);
}

SCENARIO("ConsoleSink's TextStylePolicy property should be set- and removable.", "[ConsoleSink][Sink]")
{
	ConsoleSink_t sink;

	GIVEN("an arbitrary ConsoleSink instance")
	{
		WHEN("setting TextStylePolicy")
		THEN("it should compile")
		{
			sink.setTextStylePolicy([](const Record_t&) { return ConsoleTextStyle{}; });
		}

		WHEN("removing TextStylePolicy")
		THEN("it should compile")
		{
			sink.removeTextStylePolicy();
		}
	}
}
