//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <functional>

#include "Simple-Log/Filters.hpp"
#include "Simple-Log/Record.hpp"

using namespace sl::log;

using Record_t = BaseRecord<int, int>;
using Func_t = std::function<bool(const Record_t&)>;

constexpr auto trueCb = [](auto& rec) { return true; };
constexpr auto falseCb = [](auto& rec) { return false; };
inline const Record_t record;

template <class TAlgorithm, class... TElements>
bool testAlgorithm(TElements&&... elements)
{
	auto tuple = std::tie(std::forward<TElements>(elements)...);
	return std::invoke(TAlgorithm{}, tuple, record);
}

struct VariadicAnd
{
	template <class... TElements>
	bool operator ()(TElements&&... elements) const
	{
		return (true && ... && elements(record));
	}
};

struct VariadicOr
{
	template <class... TElements>
	bool operator ()(TElements&&... elements) const
	{
		return (false || ... || elements(record));
	}
};

struct VariadicNone
{
	template <class... TElements>
	bool operator ()(TElements&&... elements) const
	{
		return !(false || ... || elements(record));
	}
};

TEMPLATE_TEST_CASE(
					"Testing tuple algorithms",
					"[Tuple_Algorithms]",
					(std::tuple<detail::TupleAllOf, VariadicAnd>),
					(std::tuple<detail::TupleAnyOf, VariadicOr>),
					(std::tuple<detail::TupleNoneOf, VariadicNone>)
				)
{
	using Algo_t = std::tuple_element_t<0, TestType>;
	using BoolCombine_t = std::tuple_element_t<1, TestType>;
	BoolCombine_t combine;

	SECTION("invoking with zero elements")
	{
		REQUIRE(testAlgorithm<Algo_t>() == combine());
	}

	SECTION("invoking with one element")
	{
		auto first = GENERATE(as<Func_t>{}, trueCb, falseCb);
		auto expectedResult = combine(first);
		REQUIRE(testAlgorithm<Algo_t>(first) == expectedResult);
	}

	SECTION("invoking with two elements")
	{
		auto first = GENERATE(as<Func_t>{}, trueCb, falseCb);
		auto second = GENERATE(as<Func_t>{}, trueCb, falseCb);
		auto expectedResult = combine(first, second);
		REQUIRE(testAlgorithm<Algo_t>(first, second) == expectedResult);
	}

	SECTION("invoking with three elements")
	{
		auto first = GENERATE(as<Func_t>{}, trueCb, falseCb);
		auto second = GENERATE(as<Func_t>{}, trueCb, falseCb);
		auto three = GENERATE(as<Func_t>{}, trueCb, falseCb);
		auto expectedResult = combine(first, second, three);
		REQUIRE(testAlgorithm<Algo_t>(first, second, three) == expectedResult);
	}

	SECTION("invoking with four elements")
	{
		auto first = GENERATE(as<Func_t>{}, trueCb, falseCb);
		auto second = GENERATE(as<Func_t>{}, trueCb, falseCb);
		auto three = GENERATE(as<Func_t>{}, trueCb, falseCb);
		auto four = GENERATE(as<Func_t>{}, trueCb, falseCb);
		auto expectedResult = combine(first, second, three, four);
		REQUIRE(testAlgorithm<Algo_t>(first, second, three, four) == expectedResult);
	}

	SECTION("invoking with five elements")
	{
		auto first = GENERATE(as<Func_t>{}, trueCb, falseCb);
		auto second = GENERATE(as<Func_t>{}, trueCb, falseCb);
		auto three = GENERATE(as<Func_t>{}, trueCb, falseCb);
		auto four = GENERATE(as<Func_t>{}, trueCb, falseCb);
		auto five = GENERATE(as<Func_t>{}, trueCb, falseCb);
		auto expectedResult = combine(first, second, three, four, five);
		REQUIRE(testAlgorithm<Algo_t>(first, second, three, four, five) == expectedResult);
	}
}
