//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <cassert>
#include <sstream>

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
