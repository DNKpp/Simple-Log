//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch2/catch.hpp"

#include <sstream>
#include <cctype>

#include "Simple-Log/StringPattern.hpp"

using namespace sl::log;

SCENARIO("numbers on demand", "[IncNumberGenerator]")
{
	detail::IncNumberGenerator incNoGen;
	std::ostringstream out;

	REQUIRE(incNoGen.current == 1);
	REQUIRE(incNoGen.minWidth == 0);

	WHEN("demanding new number")
	{
		incNoGen(out);

		THEN("current must be incremented afterwards")
		{
			REQUIRE(incNoGen.current == 2);
			REQUIRE(incNoGen.minWidth == 0);

			REQUIRE(out.str() == "1");
		}
	}

	WHEN("demanding new number with specific width")
	{
		incNoGen.minWidth = 3;
		incNoGen(out);

		THEN("current must be incremented afterwards")
		{
			REQUIRE(incNoGen.current == 2);
			REQUIRE(incNoGen.minWidth == 3);

			REQUIRE(out.str() == "001");
		}
	}
}

SCENARIO("constant strings on demand", "[IncNumberGenerStringGeneratorator]")
{
	constexpr const char* str = "Hello, World!";
	detail::StringGenerator strGen{ str };
	std::ostringstream out;

	REQUIRE(strGen.str == str);

	WHEN("demanding new string")
	{
		strGen(out);

		THEN("internal string should not change")
		{
			REQUIRE(strGen.str == str);

			REQUIRE(out.str() == str);
		}
	}
}

SCENARIO("date time on demand", "[IncNumberGenerStringGeneratorator]")
{
	detail::DateTimeGenerator dateTimeGen;
	std::ostringstream out;
	REQUIRE(dateTimeGen.token.empty());

	WHEN("demanding with empty token")
	{
		dateTimeGen(out);

		THEN("nothing will be printed")
		{
			REQUIRE(out.str().empty());
		}
	}

	WHEN("demanding with hour token")
	{
		dateTimeGen.token = "%H";
		dateTimeGen(out);

		THEN("number with width of two will be printed")
		{
			REQUIRE(std::ranges::count_if(out.str(), [](auto c) { return std::isdigit(c); }) == 2);
		}
	}

	WHEN("demanding with minute token")
	{
		dateTimeGen.token = "%M";
		dateTimeGen(out);

		THEN("number with width of two will be printed")
		{
			REQUIRE(std::ranges::count_if(out.str(), [](auto c) { return std::isdigit(c); }) == 2);
		}
	}

	WHEN("demanding with second token")
	{
		dateTimeGen.token = "%S";
		dateTimeGen(out);

		THEN("number with width of two will be printed")
		{
			REQUIRE(std::ranges::count_if(out.str(), [](auto c) { return std::isdigit(c); }) == 2);
		}
	}

	WHEN("demanding with year token")
	{
		dateTimeGen.token = "%Y";
		dateTimeGen(out);

		THEN("number with width of four will be printed")
		{
			REQUIRE(std::ranges::count_if(out.str(), [](auto c) { return std::isdigit(c); }) == 4);
		}
	}

	WHEN("demanding with month token")
	{
		dateTimeGen.token = "%m";
		dateTimeGen(out);

		THEN("number with width of two will be printed")
		{
			REQUIRE(std::ranges::count_if(out.str(), [](auto c) { return std::isdigit(c); }) == 2);
		}
	}

	WHEN("demanding with day of month token")
	{
		dateTimeGen.token = "%d";
		dateTimeGen(out);

		THEN("number with width of two will be printed")
		{
			REQUIRE(std::ranges::count_if(out.str(), [](auto c) { return std::isdigit(c); }) == 2);
		}
	}

	WHEN("demanding with day of month token")
	{
		dateTimeGen.token = "%j";
		dateTimeGen(out);

		THEN("number with width of three will be printed")
		{
			REQUIRE(std::ranges::count_if(out.str(), [](auto c) { return std::isdigit(c); }) == 3);
		}
	}
}

SCENARIO("make generator for token", "[makeGeneratorFromMatch]")
{
	WHEN("demanding with empty token string")
	{
		auto gen = detail::makeGeneratorFromMatch("");

		THEN("receiving StringGenerator with empty string")
		{
			REQUIRE(std::holds_alternative<detail::StringGenerator>(gen));
			REQUIRE(std::get<detail::StringGenerator>(gen).str.empty());
		}
	}

	WHEN("demanding with string containing no tokens")
	{
		constexpr const char* str = "Hello, World!";
		auto gen = detail::makeGeneratorFromMatch(str);

		THEN("receiving StringGenerator")
		{
			REQUIRE(std::holds_alternative<detail::StringGenerator>(gen));
			REQUIRE(std::get<detail::StringGenerator>(gen).str == str);
		}
	}

	WHEN("demanding with DateTime token")
	{
		std::string str;

		THEN("receiving DateTimeGenerator")
		{
			GIVEN("%H")
			{
				str = "%H";
			}

			GIVEN("%M")
			{
				str = "%M";
			}

			GIVEN("%S")
			{
				str = "%S";
			}

			GIVEN("%Y")
			{
				str = "%Y";
			}

			GIVEN("%m")
			{
				str = "%m";
			}

			GIVEN("%d")
			{
				str = "%d";
			}

			GIVEN("%j")
			{
				str = "%j";
			}

			auto gen = detail::makeGeneratorFromMatch(str);
			REQUIRE(std::holds_alternative<detail::DateTimeGenerator>(gen));
			REQUIRE(std::get<detail::DateTimeGenerator>(gen).token == str);
		}
	}

	WHEN("demanding with IncNumber token")
	{
		std::string str;

		THEN("receiving IncNumberGenerator")
		{
			GIVEN("%N")
			{
				str = "%N";
			}

			GIVEN("%1N")
			{
				str = "%1N";
			}

			GIVEN("%10N")
			{
				str = "%10N";
			}

			auto gen = detail::makeGeneratorFromMatch(str);
			REQUIRE(std::holds_alternative<detail::IncNumberGenerator>(gen));
		}
	}
}

SCENARIO("make generators from pattern string", "[makeTokenGeneratorsFromPatternString]")
{
	WHEN("empty string")
	{
		auto gens = detail::makeTokenGeneratorsFromPatternString("");

		THEN("no generators")
		{
			REQUIRE(std::empty(gens));
		}
	}

	WHEN("no-token string")
	{
		auto gens = detail::makeTokenGeneratorsFromPatternString("Hello, World!");

		THEN("one ConstantString generator")
		{
			REQUIRE(std::size(gens) == 1);
			REQUIRE(std::holds_alternative<detail::StringGenerator>(gens.front()));
		}
	}

	WHEN("date_time-token string")
	{
		auto gens = detail::makeTokenGeneratorsFromPatternString("Hello%M%H%S, World%Y%m%d%j!");

		THEN("ten generators")
		{
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
	}

	WHEN("IncNumber-Token string")
	{
		auto gens = detail::makeTokenGeneratorsFromPatternString("Hello%N, World%1337N!");

		THEN("five generators")
		{
			REQUIRE(std::size(gens) == 5);
			REQUIRE(std::holds_alternative<detail::StringGenerator>(gens[0]));
			REQUIRE(std::holds_alternative<detail::IncNumberGenerator>(gens[1]));
			REQUIRE(std::holds_alternative<detail::StringGenerator>(gens[2]));
			REQUIRE(std::holds_alternative<detail::IncNumberGenerator>(gens[3]));
			REQUIRE(std::holds_alternative<detail::StringGenerator>(gens[4]));
		}
	}
}

SCENARIO("generate string via pattern", "[StringPattern]")
{
	WHEN("providing constant pattern")
	{
		constexpr const char* str{ "Hello, World!" };
		StringPattern pattern{ str };
		THEN("receiving constant strings each time")
		{
			REQUIRE(pattern.next() == str);
			REQUIRE(pattern.next() == str);
			REQUIRE(pattern.next() == str);
		}
	}

	WHEN("providing inc number pattern")
	{
		using namespace std::string_literals;
		constexpr const char* str{ "Hello, World!" };
		StringPattern pattern{ str + "%N"s };
		THEN("receiving updated strings each time")
		{
			REQUIRE(pattern.next() == str + "1"s);
			REQUIRE(pattern.next() == str + "2"s);
			REQUIRE(pattern.next() == str + "3"s);
		}
	}
}
