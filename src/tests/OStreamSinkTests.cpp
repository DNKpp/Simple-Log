//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include "Simple-Log/OStreamSink.hpp"
#include "Simple-Log/Record.hpp"

using namespace sl::log;

using Record_t = BaseRecord<int, int>;
using OStreamSink_t = OStreamSink<Record_t>;

namespace
{
	struct FlushPolicyMoc
	{
		bool operator ()(const Record_t& rec, std::size_t byteCount) const noexcept
		{
			assert(invocationCount);
			++(*invocationCount);
			return (*invocationCount & 1ull) != 0;
		}

		void flushed() const noexcept
		{
			assert(flushedSignalCount);
			++(*flushedSignalCount);
		}

		std::size_t* invocationCount;
		std::size_t* flushedSignalCount;
	};

	struct FormatterMoc
	{
		std::string operator ()(const Record_t& rec) const
		{
			assert(invoked);
			*invoked = true;
			return std::string(overridingStr);
		}

		std::string_view overridingStr;
		bool* invoked;
	};

	struct FilterMoc
	{
		bool operator ()(const Record_t& rec) const
		{
			assert(invoked);
			*invoked = true;
			return invocationResult;
		}

		bool invocationResult;
		bool* invoked;
	};
}

SCENARIO("OStreamSinks should be in disabled state when construction succeeded", "[OStreamSink][Sink]")
{
	GIVEN("a newly constructed OStreamSink instance")
	WHEN("construction succeeded")
	THEN("OStreamSink should be disabled by default")
	{
		OStreamSink_t sink{ std::cout };
		REQUIRE_FALSE(sink.isEnabled());
	}
}

SCENARIO("OStreamSinks::setEnabled should modify member.", "[OStreamSink][Sink]")
{
	GIVEN("a disabled OStreamSink instance")
	WHEN("setting as enabled")
	THEN("isEnabled should yield a positive result")
	{
		OStreamSink_t sink{ std::cout };
		sink.setEnabled(true);
		REQUIRE(sink.isEnabled());
	}
}

SCENARIO("OStreamSink's log function should be controled by enabled property", "[OStreamSink][Sink]")
{
	std::ostringstream out;
	OStreamSink_t sink{ out };

	GIVEN("a disabled OStreamSink instance")
	WHEN("calling log")
	THEN("nothing should have been written to the stream")
	{
		sink.log({});
		REQUIRE(std::empty(out.str()));
	}

	GIVEN("a enabled OStreamSink instance")
	WHEN("calling log")
	THEN("record should have been written to the stream")
	{
		sink.setEnabled(true);
		sink.log({});
		REQUIRE_FALSE(std::empty(out.str()));
	}
}

SCENARIO("OStreamSink's filter property should determine if records get processed or skipped", "[OStreamSink][Sink]")
{
	std::ostringstream out;
	OStreamSink_t sink{ out };
	bool invoked = false;
	sink.setFilter(FilterMoc{ .invocationResult = true, .invoked = &invoked });
	sink.setEnabled();

	GIVEN("an enabled OStreamSink instance")
	{
		WHEN("calling log")
		{
			THEN("filter property should get invoked ")
			{
				sink.log({});
				REQUIRE(invoked);
			}

			AND_THEN("filter should determine if record gets handled")
			{
				auto shallHandle = GENERATE(true, false);
				sink.setFilter(FilterMoc{ .invocationResult = shallHandle, .invoked = &invoked });
				sink.log({});
				REQUIRE(std::empty(out.str()) != shallHandle);
			}
		}

		AND_WHEN("removeFilter is called")
		THEN("then previous filter should get replaced")
		{
			sink.removeFilter();
			sink.log({});
			REQUIRE_FALSE(invoked);
		}
	}
}

SCENARIO("OStreamSink's formatter property should format processed records", "[OStreamSink][Sink]")
{
	std::ostringstream out;
	OStreamSink_t sink{ out };
	bool invoked = false;
	sink.setFormatter(FormatterMoc{ .invoked = &invoked });
	sink.setEnabled();

	GIVEN("an enabled OStreamSink instance")
	{
		WHEN("calling log")
		{
			THEN("formatter property should get invoked ")
			{
				sink.log({});
				REQUIRE(invoked);
			}

			AND_THEN("formatter should format records")
			{
				const auto overridingStr = GENERATE(as<std::string>{}, "", "Hello, World!");
				sink.setFormatter(FormatterMoc{ .overridingStr = overridingStr, .invoked = &invoked });
				sink.log({});
				REQUIRE(out.str() == overridingStr + "\n");
			}
		}

		AND_WHEN("removeFormatter is called")
		THEN("then previous formatter should get replaced")
		{
			sink.removeFormatter();
			sink.log({});
			REQUIRE_FALSE(invoked);
		}
	}
}

SCENARIO("OStreamSink's FlushPolicy property should determine when stream has to be flushed.", "[OStreamSink][Sink]")
{
	std::ostringstream out;
	OStreamSink_t sink{ out };
	sink.setEnabled();

	std::size_t invocationCount = 0;
	std::size_t flushedSignalCount = 0;

	sink.setFlushPolicy(
						FlushPolicyMoc{
							.invocationCount = &invocationCount,
							.flushedSignalCount = &flushedSignalCount
						}
						);

	GIVEN("an enabled OStreamSink instance")
	{
		WHEN("calling log")
		{
			sink.log({});
			THEN("FlushPolicy property should get invoked")
			{
				REQUIRE(invocationCount == 1);
			}

			AND_THEN("FlushPolicy property should get notified about flushed")
			{
				REQUIRE(flushedSignalCount == 1);
			}
		}

		AND_WHEN("calling flushed")
		AND_THEN("FlushPolicy property should get notified about flushed")
		{
			sink.flush();
			REQUIRE(flushedSignalCount == 1);
		}

		AND_WHEN("removeFlushPolicy is called")
		THEN("then previous FlushPolicy should get replaced")
		{
			sink.removeFlushPolicy();
			sink.log({});
			REQUIRE(invocationCount == 0);
		}
	}
}
