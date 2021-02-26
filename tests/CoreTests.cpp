//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <iostream>

#include "Simple-Log/OStreamSink.hpp"
#include "Simple-Log/Core.hpp"
#include "Simple-Log/Record.hpp"

using namespace sl::log;

using Record_t = BaseRecord<int, int>;
using Core_t = Core<Record_t>;
using Sink_t = OStreamSink<Record_t>;

SCENARIO("Core should create valid Sink instances.", "[Core]")
{
	Core_t core;

	GIVEN("an arbitrary Core instance")
	{
		WHEN("makeSink is called")
		THEN("caller receives a reference to a valid enabled Sink object")
		{
			auto& sink = core.makeSink<Sink_t>(std::cout);

			REQUIRE(sink.isEnabled());
		}

		WHEN("makeDisabledSink is called")
		THEN("caller receives a wrapped reference to a valid disabled Sink object")
		{
			Sink_t* sinkPtr = nullptr;
			{
				auto wrappedSink = core.makeDisabledSink<Sink_t>(std::cout);

				REQUIRE(wrappedSink.get() != nullptr);
				REQUIRE_FALSE(wrappedSink->isEnabled());

				sinkPtr = wrappedSink.get();
			}

			AND_WHEN("reference wrapper goes out of scope")
			THEN("Sink object will automaticalle become enabled")
			{
				REQUIRE(sinkPtr->isEnabled());
			}
		}
	}
}

SCENARIO("Core should remove registered Sink objects.", "[Core]")
{
	Core_t core;

	GIVEN("an arbitrary Core instance")
	{
		WHEN("removeSink with an registered Sink is called")
		THEN("Sink object gets destroyed and function yields true")
		{
			auto& sink = core.makeSink<Sink_t>(std::cout);

			REQUIRE(core.removeSink(sink));
		}

		WHEN("removeSink with an un-registered Sink is called")
		THEN("Sink object won't get destroyed and function yields false")
		{
			Sink_t sink{ std::cout };

			REQUIRE_FALSE(core.removeSink(sink));
		}
	}
}
