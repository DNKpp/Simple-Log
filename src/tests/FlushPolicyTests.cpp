//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch2/catch.hpp"

#include <random>
#include <thread>

#include "Simple-Log/FlushPolicies.hpp"
#include "Simple-Log/Record.hpp"
#include "Simple-Log/Predicates.hpp"

using namespace sl::log;

using Record_t = BaseRecord<int, int>;

TEST_CASE("ConstInvokable", "[Flush-Policies]")
{
	detail::ConstantInvokable<true> alwaysTrue;
	REQUIRE(alwaysTrue());

	detail::ConstantInvokable<false> alwaysFalse;
	REQUIRE_FALSE(alwaysFalse());

	detail::ConstantInvokable<1337> always1337;
	REQUIRE(always1337() == 1337);
}

TEST_CASE("FlushPolicy", "[Flush-Policies]")
{
	int counter = 5;
	auto countingDown = [counter](const auto& rec, std::size_t byteCount) mutable
	{
		return --counter < 0;
	};

	FlushPolicy policy{ countingDown };
	Record_t dummy;
	for (int i = 0; i < counter; ++i)
	{
		REQUIRE_FALSE(policy(dummy, 0));
	}
	REQUIRE(policy(dummy, 0));
}

TEST_CASE("AlwaysFlushPolicy", "[Flush-Policies]")
{
	AlwaysFlushPolicy policy{};
	Record_t dummy;
	for (int i = 0; i < 10; ++i)
	{
		REQUIRE(policy(dummy, 0));
	}
}

TEST_CASE("makeSeverityFlushPolicyFor", "[Flush-Policies]")
{
	auto policy = makeSeverityFlushPolicyFor<Record_t>(Less{ 5 });
	Record_t record;
	for (int i = 0; i < 5; ++i)
	{
		record.setSeverity(i);
		REQUIRE(policy(record, 0));
	}

	for (int i = 6; i < 10; ++i)
	{
		record.setSeverity(i);
		REQUIRE_FALSE(policy(record, 0));
	}
}

TEST_CASE("makeChannelFlushPolicyFor", "[Flush-Policies]")
{
	auto policy = makeChannelFlushPolicyFor<Record_t>(Less{ 5 });
	Record_t record;
	for (int i = 0; i < 5; ++i)
	{
		record.setChannel(i);
		REQUIRE(policy(record, 0));
	}

	for (int i = 6; i < 10; ++i)
	{
		record.setChannel(i);
		REQUIRE_FALSE(policy(record, 0));
	}
}

TEST_CASE("makeTimePointFlushPolicyFor", "[Flush-Policies]")
{
	using Record2_t = BaseRecord<int, int, int, int>;
	auto policy = makeTimePointFlushPolicyFor<Record2_t>(Less{ 5 });
	Record2_t record;
	for (int i = 0; i < 5; ++i)
	{
		record.setTimePoint(i);
		REQUIRE(policy(record, 0));
	}

	for (int i = 6; i < 10; ++i)
	{
		record.setTimePoint(i);
		REQUIRE_FALSE(policy(record, 0));
	}
}


TEST_CASE("ByteCountFlushPolicy", "[Flush-Policies]")
{
	const std::size_t threshold = 10 * 1024;
	ByteCountFlushPolicy policy{ threshold };
	Record_t dummy;

	SECTION("message with constant 1024 bytes")
	{
		for (int j = 0; j < 3; ++j)
		{
			for (int i = 0; i < 9; ++i)
			{
				REQUIRE_FALSE(policy(dummy, 1024));
			}
			REQUIRE(policy(dummy, 1024));
			policy.flushed();
		}
	}

	SECTION("message with varying bytes")
	{
		std::size_t byteCount = 0;
		std::mt19937 rng;
		std::uniform_int_distribution<std::size_t> distrib{ 1, 512 };

		for (int i = 0; i < 3; ++i)
		{
			auto nextBytes = distrib(rng);
			while (byteCount + nextBytes < threshold)
			{
				byteCount += nextBytes;
				REQUIRE_FALSE(policy(dummy, nextBytes));
				nextBytes = distrib(rng);
			}
			REQUIRE(policy(dummy, nextBytes));
			policy.flushed();
			REQUIRE_FALSE(policy(dummy, 0));
			byteCount = 0;
		}
	}
}

SCENARIO("TimedFlushPolicy", "[Flush-Policies]")
{
	const std::chrono::milliseconds threshold{ 250 };
	TimedFlushPolicy policy{ threshold * 2 };
	Record_t dummy;

	WHEN("duration between flushes is at 0.5sec")
	{
		AND_WHEN("invoking half of the time with random byte count")
		{
			THEN("always returns false")
			{
				std::mt19937 rng;
				std::uniform_int_distribution<std::size_t> distrib{ 1, 512 };
				const auto begin = TimedFlushPolicy::Clock_t::now();
				while (threshold < TimedFlushPolicy::Clock_t::now() - begin)
				{
					REQUIRE_FALSE(policy(dummy, distrib(rng)));
				}
			}
		}

		AND_WHEN("waiting at least that duration")
		{
			std::this_thread::sleep_for(threshold * 2);
			THEN("returns true")
			{
				REQUIRE(policy(dummy, 1));

				AND_THEN("again returns false")
				{
					std::mt19937 rng;
					std::uniform_int_distribution<std::size_t> distrib{ 1, 512 };
					const auto begin = TimedFlushPolicy::Clock_t::now();
					while (threshold < TimedFlushPolicy::Clock_t::now() - begin)
					{
						REQUIRE_FALSE(policy(dummy, distrib(rng)));
					}
				}
			}
		}
	}
}
