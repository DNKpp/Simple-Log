//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch2/catch.hpp"

#include "Simple-Log/Predicates.hpp"

using namespace sl::log;

SCENARIO("test equality comparing predicates", "[equality-predicates]")
{
	WHEN("testing for equality")
	{
		int value = 1337;
		EqualsTo<int> equals{ value };
		GIVEN("equal value")
		{
			THEN("returns true")
			{
				REQUIRE(equals(value));
			}
		}

		GIVEN("non-equal value")
		{
			value = 42;
			THEN("returns false")
			{
				REQUIRE_FALSE(equals(value));
			}
		}
	}

	WHEN("testing for non-equality")
	{
		int value = 1337;
		NotEquals<int> notEquals{ value };
		GIVEN("equal value")
		{
			THEN("returns false")
			{
				REQUIRE_FALSE(notEquals(value));
			}
		}

		GIVEN("non-equal value")
		{
			value = 42;
			THEN("returns false")
			{
				REQUIRE(notEquals(value));
			}
		}
	}
}

SCENARIO("test less ordering predicates", "[total-ordering-predicates]")
{
	WHEN("testing for less")
	{
		int value = 1337;
		Less<int> less{ value };
		GIVEN("equal value")
		{
			THEN("returns false")
			{
				REQUIRE_FALSE(less(value));
			}
		}

		GIVEN("lesser value")
		{
			--value;
			THEN("returns true")
			{
				REQUIRE(less(value));
			}
		}

		GIVEN("greater value")
		{
			++value;
			THEN("returns false")
			{
				REQUIRE_FALSE(less(value));
			}
		}
	}

	WHEN("testing for less-equals")
	{
		int value = 1337;
		LessEquals<int> lessEquals{ value };
		GIVEN("equal value")
		{
			THEN("returns true")
			{
				REQUIRE(lessEquals(value));
			}
		}

		GIVEN("lesser value")
		{
			--value;
			THEN("returns true")
			{
				REQUIRE(lessEquals(value));
			}
		}

		GIVEN("greater value")
		{
			++value;
			THEN("returns true")
			{
				REQUIRE_FALSE(lessEquals(value));
			}
		}
	}
}

SCENARIO("test greater ordering predicates", "[total-ordering-predicates]")
{
	WHEN("testing for greater")
	{
		int value = 1337;
		Greater<int> greater{ value };
		GIVEN("equal value")
		{
			THEN("returns false")
			{
				REQUIRE_FALSE(greater(value));
			}
		}

		GIVEN("lesser value")
		{
			--value;
			THEN("returns false")
			{
				REQUIRE_FALSE(greater(value));
			}
		}

		GIVEN("greater value")
		{
			++value;
			THEN("returns true")
			{
				REQUIRE(greater(value));
			}
		}
	}

	WHEN("testing for greater-equals")
	{
		int value = 1337;
		GreaterEqual<int> greaterEquals{ value };
		GIVEN("equal value")
		{
			THEN("returns true")
			{
				REQUIRE(greaterEquals(value));
			}
		}

		GIVEN("lesser value")
		{
			--value;
			THEN("returns false")
			{
				REQUIRE_FALSE(greaterEquals(value));
			}
		}

		GIVEN("greater value")
		{
			++value;
			THEN("returns true")
			{
				REQUIRE(greaterEquals(value));
			}
		}
	}
}

SCENARIO("test between values predicate", "[total-ordering-predicates]")
{
	WHEN("testing for between two equal values")
	{
		int value = 1337;
		Between<int> between{ value, value };
		GIVEN("equal values")
		{
			THEN("returns false")
			{
				REQUIRE_FALSE(between(value));
			}
		}

		GIVEN("lesser value")
		{
			--value;
			THEN("returns false")
			{
				REQUIRE_FALSE(between(value));
			}
		}

		GIVEN("greater value")
		{
			++value;
			THEN("returns false")
			{
				REQUIRE_FALSE(between(value));
			}
		}
	}
	
	WHEN("testing for between two distinct values")
	{
		int value = 1337;
		Between<int> between{ value, value + 2 };
		GIVEN("value equals low")
		{
			THEN("returns false")
			{
				REQUIRE_FALSE(between(value));
			}
		}

		GIVEN("value equals high")
		{
			value += 2;
			THEN("returns false")
			{
				REQUIRE_FALSE(between(value));
			}
		}

		GIVEN("value between low and high")
		{
			value += 1;
			THEN("returns true")
			{
				REQUIRE(between(value));
			}
		}

		GIVEN("lesser than low")
		{
			--value;
			THEN("returns false")
			{
				REQUIRE_FALSE(between(value));
			}
		}

		GIVEN("greater value")
		{
			value += 3;
			THEN("returns false")
			{
				REQUIRE_FALSE(between(value));
			}
		}
	}
}

SCENARIO("test between-equals values predicate", "[total-ordering-predicates]")
{
	WHEN("testing for between two equal values")
	{
		int value = 1337;
		BetweenEquals<int> between{ value, value };
		GIVEN("equal values")
		{
			THEN("returns true")
			{
				REQUIRE(between(value));
			}
		}

		GIVEN("lesser value")
		{
			--value;
			THEN("returns false")
			{
				REQUIRE_FALSE(between(value));
			}
		}

		GIVEN("greater value")
		{
			++value;
			THEN("returns false")
			{
				REQUIRE_FALSE(between(value));
			}
		}
	}
	
	WHEN("testing for between two distrinct values")
	{
		int value = 1337;
		BetweenEquals<int> between{ value, value + 2 };
		GIVEN("value equals low")
		{
			THEN("returns true")
			{
				REQUIRE(between(value));
			}
		}

		GIVEN("value equals high")
		{
			value += 2;
			THEN("returns true")
			{
				REQUIRE(between(value));
			}
		}

		GIVEN("value between low and high")
		{
			value += 1;
			THEN("returns true")
			{
				REQUIRE(between(value));
			}
		}

		GIVEN("lesser than low")
		{
			--value;
			THEN("returns false")
			{
				REQUIRE_FALSE(between(value));
			}
		}

		GIVEN("greater value")
		{
			value += 3;
			THEN("returns false")
			{
				REQUIRE_FALSE(between(value));
			}
		}
	}
}