//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <regex>
#include <sstream>

#include "Simple-Log/StringPattern.hpp"

using namespace sl::log;
using namespace Catch::Matchers;

TEST_CASE("IncNumberGenerator generates consecutive numbers of minimum given width", "[StringPattern]")
{
	detail::IncNumberGenerator incNoGen;

	REQUIRE(incNoGen.current == 1);
	REQUIRE(incNoGen.minWidth == 0);

	auto [current, width, expectedStr] = GENERATE(
												table<unsigned,
												unsigned,
												std::string_view>(
													{
													{ 1, 0, "1" },
													{ 1, 1, "1" },
													{ 1, 2, "01" },
													{ 10, 0, "10" },
													{ 10, 3, "010" }
													}
												)
												);

	incNoGen.current = current;
	incNoGen.minWidth = width;

	std::ostringstream out;
	incNoGen(out);

	REQUIRE(out.str() == expectedStr);
	REQUIRE(incNoGen.current == current + 1);
	REQUIRE(incNoGen.minWidth == width);
}

TEST_CASE("StringGenerator generates constant string each time", "[StringPattern]")
{
	detail::StringGenerator strGen;
	REQUIRE(std::empty(strGen.str));

	auto str = GENERATE("", " ", "test", "Hello, World!");
	strGen.str = str;

	std::ostringstream out;
	strGen(out);

	REQUIRE(out.str() == str);
	REQUIRE(strGen.str == str);
}

TEST_CASE("DateTimeGenerator generates string from current date-time", "[StringPattern]")
{
	detail::DateTimeGenerator dateTimeGen;
	REQUIRE(std::empty(dateTimeGen.token));

	auto [token, digits] = GENERATE(
									table<std::string_view,
									std::size_t>(
										{
										{ "", 0 },
										{"%H", 2},
										{"%M", 2},
										{"%S", 2},
										{"%Y", 4},
										{"%m", 2},
										{"%d", 2},
										{"%j", 3}
										})
									);
	dateTimeGen.token = token;

	for (std::size_t i = 0; i < 5; ++i)
	{
		std::ostringstream out;
		dateTimeGen(out);

		REQUIRE(dateTimeGen.token == token);
		REQUIRE_THAT(out.str(), Matches("\\d{" + std::to_string(digits) + "}"));
	}
}

TEST_CASE("makeGeneratorFromMatch makes generators by token", "[StringPattern]")
{
	SECTION("generating from constant string")
	{
		auto str = GENERATE("", " ", "test", "Hello, World!");
		auto gen = detail::makeGeneratorFromMatch(str);

		auto actualGen = std::get_if<detail::StringGenerator>(&gen);
		REQUIRE(actualGen);
		REQUIRE(actualGen->str == str);
	}

	auto checkIncNumberGen = [](const auto& variant, std::size_t width)
	{
		auto actualGen = std::get_if<detail::IncNumberGenerator>(&variant);
		REQUIRE(actualGen);
		REQUIRE(actualGen->current == 1);
		REQUIRE(actualGen->minWidth == width);
	};

	SECTION("generating via %N token")
	{
		checkIncNumberGen(detail::makeGeneratorFromMatch("%N"), 0);
	}

	SECTION("generating via variations of %XN token")
	{
		auto width = GENERATE(0, 1, take(5, random(0, 100)));
		checkIncNumberGen(detail::makeGeneratorFromMatch("%" + std::to_string(width) + "N"), width);
	}

	SECTION("generating via date-time %XN token")
	{
		auto token = GENERATE("%H", "%M", "%S", "%Y", "%m", "%d", "%j");
		auto gen = detail::makeGeneratorFromMatch(token);

		auto actualGen = std::get_if<detail::DateTimeGenerator>(&gen);
		REQUIRE(actualGen);
		REQUIRE(actualGen->token == token);
	}
}

TEST_CASE("makeTokenGeneratorsFromPatternString makes one or multiple generators from string pattern", "[StringPattern]")
{
	SECTION("generate from empty string")
	{
		auto gens = detail::makeTokenGeneratorsFromPatternString("");
		REQUIRE(std::empty(gens));
	}

	SECTION("generating from string without any token")
	{
		auto gens = detail::makeTokenGeneratorsFromPatternString("Hello, World!");
		REQUIRE(std::size(gens) == 1);
		REQUIRE(std::holds_alternative<detail::StringGenerator>(gens.front()));
	}

	SECTION("generating from string with mixed date-time and string tokens")
	{
		auto gens = detail::makeTokenGeneratorsFromPatternString("Hello%M%H%S, World%Y%m%d%j!");
		REQUIRE(std::size(gens) == 10);
		REQUIRE(std::holds_alternative<detail::StringGenerator>(gens[0]));
		REQUIRE(std::holds_alternative<detail::DateTimeGenerator>(gens[1]));
		REQUIRE(std::holds_alternative<detail::DateTimeGenerator>(gens[2]));
		REQUIRE(std::holds_alternative<detail::DateTimeGenerator>(gens[3]));
		REQUIRE(std::holds_alternative<detail::StringGenerator>(gens[4]));
		REQUIRE(std::holds_alternative<detail::DateTimeGenerator>(gens[5]));
		REQUIRE(std::holds_alternative<detail::DateTimeGenerator>(gens[6]));
		REQUIRE(std::holds_alternative<detail::DateTimeGenerator>(gens[7]));
		REQUIRE(std::holds_alternative<detail::DateTimeGenerator>(gens[8]));
		REQUIRE(std::holds_alternative<detail::StringGenerator>(gens[9]));
	}

	SECTION("generating from string with mixed inc-number and string tokens")
	{
		auto gens = detail::makeTokenGeneratorsFromPatternString("Hello%N, World%1337N!");
		REQUIRE(std::size(gens) == 5);
		REQUIRE(std::holds_alternative<detail::StringGenerator>(gens[0]));
		REQUIRE(std::holds_alternative<detail::IncNumberGenerator>(gens[1]));
		REQUIRE(std::holds_alternative<detail::StringGenerator>(gens[2]));
		REQUIRE(std::holds_alternative<detail::IncNumberGenerator>(gens[3]));
		REQUIRE(std::holds_alternative<detail::StringGenerator>(gens[4]));
	}
}

TEST_CASE("StringPattern invocation produces strings, which will be generated on the fly on each invokation", "[StringPattern]")
{
	SECTION("generating from constant pattern")
	{
		auto str = GENERATE("", " ", "test", "Hello, World!");
		StringPattern pattern{ str };

		for (std::size_t i = 0; i < 5; ++i)
		{
			REQUIRE(pattern.next() == str);
		}
	}

	SECTION("generating from string with tokens")
	{
		using namespace std::string_literals;
		auto pattern = GENERATE("%N", ".%N", "%Ntest%N", "%NHello%N%N,World!%N");
		StringPattern patternedGen{ pattern };
		std::regex regex{ "%N" };
		for (std::size_t i = 1; i <= 5; ++i)
		{
			auto expectedStr = std::regex_replace(
												pattern,
												regex,
												std::to_string(i)
												);
			REQUIRE(patternedGen.next() == expectedStr);
		}
	}
}
