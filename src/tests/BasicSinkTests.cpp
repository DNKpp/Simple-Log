//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <cassert>
#include <sstream>
#include <string>
#include <string_view>

#include "Simple-Log/BasicSink.hpp"
#include "Simple-Log/Record.hpp"

using namespace sl::log;

using Record_t = BaseRecord<int, int>;

namespace
{
	class BasicSinkMoc :
		public BasicSink<Record_t>
	{
	public:
		BasicSinkMoc(std::ostream* out = nullptr) :
			m_Out{ out }
		{
		}

	private:
		void writeMessage(const Record_t& record, std::string_view message) override
		{
			if (m_Out)
			{
				(*m_Out) << message << "\n";
			}
		}

		std::ostream* m_Out = nullptr;
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

SCENARIO("BasicSinks should be in disabled state when construction succeeded", "[BasicSink][Sink]")
{
	GIVEN("a newly constructed BasicSink instance")
	WHEN("construction succeeded")
	THEN("BasicSink should be disabled by default")
	{
		BasicSinkMoc sink;
		REQUIRE_FALSE(sink.isEnabled());
	}
}

SCENARIO("BasicSinks::setEnabled should modify member.", "[BasicSink][Sink]")
{
	GIVEN("a disabled BasicSink instance")
	WHEN("setting as enabled")
	THEN("isEnabled should yield a positive result")
	{
		BasicSinkMoc sink;
		sink.setEnabled(true);
		REQUIRE(sink.isEnabled());
	}
}

SCENARIO("BasicSink's log function should be controled by enabled property", "[BasicSink][Sink]")
{
	std::ostringstream out;
	BasicSinkMoc sink{ &out };

	GIVEN("a disabled BasicSink instance")
	WHEN("calling log")
	THEN("nothing should have been written to the stream")
	{
		sink.log({});
		REQUIRE(std::empty(out.str()));
	}

	GIVEN("a enabled BasicSink instance")
	WHEN("calling log")
	THEN("record should have been written to the stream")
	{
		sink.setEnabled(true);
		sink.log({});
		REQUIRE_FALSE(std::empty(out.str()));
	}
}

SCENARIO("BasicSink's filter property should determine if records get processed or skipped", "[BasicSink][Sink]")
{
	std::ostringstream out;
	BasicSinkMoc sink{ &out };
	bool invoked = false;
	sink.setFilter(FilterMoc{ .invocationResult = true, .invoked = &invoked });
	sink.setEnabled();

	GIVEN("an enabled BasicSink instance")
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

SCENARIO("BasicSink's formatter property should format processed records", "[BasicSink][Sink]")
{
	std::ostringstream out;
	BasicSinkMoc sink{ &out };
	bool invoked = false;
	sink.setFormatter(FormatterMoc{ .invoked = &invoked });
	sink.setEnabled();

	GIVEN("an enabled BasicSink instance")
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
