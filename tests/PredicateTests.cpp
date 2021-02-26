//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include "Simple-Log/Predicates.hpp"

using namespace sl::log;

TEMPLATE_TEST_CASE_SIG(
						"Single-bounded Unary-Comparison-Predicates should yield expected results on invocation",
						"[Total-Ordering][Predicates]",
						((typename TPred, auto Init, auto Value, bool Expected), TPred, Init, Value, Expected),
						(Equals<int>, 1337, 1337, true),
						(Equals<int>, 1337, 42, false),

						(NotEquals<int>, 1337, 1337, false),
						(NotEquals<int>, 1337, 42, true),

						(Less<int>, 1337, 1338, false),
						(Less<int>, 1337, 1337, false),
						(Less<int>, 1337, 1336, true),

						(LessEquals<int>, 1337, 1338, false),
						(LessEquals<int>, 1337, 1337, true),
						(LessEquals<int>, 1337, 1336, true),

						(Greater<int>, 1337, 1338, true),
						(Greater<int>, 1337, 1337, false),
						(Greater<int>, 1337, 1336, false),

						(GreaterEquals<int>, 1337, 1338, true),
						(GreaterEquals<int>, 1337, 1337, true),
						(GreaterEquals<int>, 1337, 1336, false)
					)
{
	TPred predicate{ Init };

	auto result = std::invoke(predicate, Value);

	REQUIRE(result == Expected);
}

TEMPLATE_TEST_CASE_SIG(
						"Dual-bounded Unary-Comparison-Predicates should yield excpeted results on invocation",
						"[Total-Ordering][Predicates]",
						((typename TPred, auto Init1, auto Init2, auto Value, bool Expected),
							TPred, Init1, Init2, Value, Expected),

						(Between<int>, 1337, 1337, 1338, false),
						(Between<int>, 1337, 1337, 1337, false),
						(Between<int>, 1337, 1337, 1336, false),

						(Between<int>, 1336, 1337, 1338, false),
						(Between<int>, 1336, 1337, 1337, false),
						(Between<int>, 1336, 1337, 1336, false),
						(Between<int>, 1336, 1337, 1335, false),

						(Between<int>, 1336, 1338, 1339, false),
						(Between<int>, 1336, 1338, 1338, false),
						(Between<int>, 1336, 1338, 1337, true),
						(Between<int>, 1336, 1338, 1336, false),
						(Between<int>, 1336, 1338, 1335, false),

						(BetweenEquals<int>, 1337, 1337, 1338, false),
						(BetweenEquals<int>, 1337, 1337, 1337, true),
						(BetweenEquals<int>, 1337, 1337, 1336, false),

						(BetweenEquals<int>, 1336, 1337, 1338, false),
						(BetweenEquals<int>, 1336, 1337, 1337, true),
						(BetweenEquals<int>, 1336, 1337, 1336, true),
						(BetweenEquals<int>, 1336, 1337, 1335, false),

						(BetweenEquals<int>, 1336, 1338, 1339, false),
						(BetweenEquals<int>, 1336, 1338, 1338, true),
						(BetweenEquals<int>, 1336, 1338, 1337, true),
						(BetweenEquals<int>, 1336, 1338, 1336, true),
						(BetweenEquals<int>, 1336, 1338, 1335, false)
					)
{
	TPred predicate{ Init1, Init2 };

	auto result = std::invoke(predicate, Value);

	REQUIRE(result == Expected);
}
