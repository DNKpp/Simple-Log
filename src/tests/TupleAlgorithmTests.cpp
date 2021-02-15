//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch2/catch.hpp"

#include "Simple-Log/Filters.hpp"
#include "Simple-Log/PresetTypes.hpp"

using namespace sl::log;

inline auto trueCb = [](auto& rec) { return true;  };
inline auto falseCb = [](auto& rec) { return false; };

SCENARIO("TupleAllOf", "[filters/tuple-algorithms]")
{	
	detail::TupleAllOf algorithm;
	pre::Record_t record;
	
	GIVEN("empty tuple")
	{
		std::tuple<> tuple;

		THEN("algorithm returns true")
		{
			REQUIRE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with one constant true element")
	{
		std::tuple tuple{ trueCb };

		THEN("algorithm returns true")
		{
			REQUIRE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with three true element")
	{
		std::tuple tuple{ trueCb, trueCb, trueCb };

		THEN("algorithm returns true")
		{
			REQUIRE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with one constant false element")
	{
		std::tuple tuple{ falseCb };

		THEN("algorithm returns false")
		{
			REQUIRE_FALSE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with three but first false element")
	{
		std::tuple tuple{ falseCb, trueCb, trueCb };

		THEN("algorithm returns false")
		{
			REQUIRE_FALSE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with three but last false element")
	{
		std::tuple tuple{ trueCb, trueCb, falseCb };

		THEN("algorithm returns false")
		{
			REQUIRE_FALSE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with three but mid false element")
	{
		std::tuple tuple{ trueCb, falseCb, trueCb };

		THEN("algorithm returns false")
		{
			REQUIRE_FALSE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with three false element")
	{
		std::tuple tuple{ falseCb, falseCb, falseCb };

		THEN("algorithm returns false")
		{
			REQUIRE_FALSE(algorithm(tuple, record));
		}
	}
}


SCENARIO("TupleAnyOf", "[filters/tuple-algorithms]")
{	
	detail::TupleAnyOf algorithm;
	pre::Record_t record;
	
	GIVEN("empty tuple")
	{
		std::tuple<> tuple;

		THEN("algorithm returns false")
		{
			REQUIRE_FALSE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with one constant true element")
	{
		std::tuple tuple{ trueCb };

		THEN("algorithm returns true")
		{
			REQUIRE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with three true element")
	{
		std::tuple tuple{ trueCb, trueCb, trueCb };

		THEN("algorithm returns true")
		{
			REQUIRE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with one constant false element")
	{
		std::tuple tuple{ falseCb };

		THEN("algorithm returns false")
		{
			REQUIRE_FALSE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with three but first false element")
	{
		std::tuple tuple{ falseCb, trueCb, trueCb };

		THEN("algorithm returns true")
		{
			REQUIRE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with three but last false element")
	{
		std::tuple tuple{ trueCb, trueCb, falseCb };

		THEN("algorithm returns true")
		{
			REQUIRE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with three but mid false element")
	{
		std::tuple tuple{ trueCb, falseCb, trueCb };

		THEN("algorithm returns true")
		{
			REQUIRE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with three false element")
	{
		std::tuple tuple{ falseCb, falseCb, falseCb };

		THEN("algorithm returns false")
		{
			REQUIRE_FALSE(algorithm(tuple, record));
		}
	}
}

SCENARIO("TupleNoneOf", "[filters/tuple-algorithms]")
{	
	detail::TupleNoneOf algorithm;
	pre::Record_t record;
	
	GIVEN("empty tuple")
	{
		std::tuple<> tuple;

		THEN("algorithm returns true")
		{
			REQUIRE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with one constant true element")
	{
		std::tuple tuple{ trueCb };

		THEN("algorithm returns false")
		{
			REQUIRE_FALSE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with three true element")
	{
		std::tuple tuple{ trueCb, trueCb, trueCb };

		THEN("algorithm returns false")
		{
			REQUIRE_FALSE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with one constant false element")
	{
		std::tuple tuple{ falseCb };

		THEN("algorithm returns true")
		{
			REQUIRE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with three but first false element")
	{
		std::tuple tuple{ falseCb, trueCb, trueCb };

		THEN("algorithm returns false")
		{
			REQUIRE_FALSE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with three but last false element")
	{
		std::tuple tuple{ trueCb, trueCb, falseCb };

		THEN("algorithm returns false")
		{
			REQUIRE_FALSE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with three but mid false element")
	{
		std::tuple tuple{ trueCb, falseCb, trueCb };

		THEN("algorithm returns false")
		{
			REQUIRE_FALSE(algorithm(tuple, record));
		}
	}

	GIVEN("tuple with three false element")
	{
		std::tuple tuple{ falseCb, falseCb, falseCb };

		THEN("algorithm returns true")
		{
			REQUIRE(algorithm(tuple, record));
		}
	}
}
