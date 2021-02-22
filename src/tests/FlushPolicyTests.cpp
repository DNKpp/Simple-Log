//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <thread>

#include "Simple-Log/FlushPolicies.hpp"
#include "Simple-Log/Predicates.hpp"
#include "Simple-Log/Record.hpp"

using namespace sl::log;

using Record_t = BaseRecord<int, int, int, int>;

TEST_CASE("ConstInvokable should always return the expected result", "[Flush-Policy]")
{
	detail::ConstantInvokable<true> alwaysTrue;
	REQUIRE(alwaysTrue());

	detail::ConstantInvokable<false> alwaysFalse;
	REQUIRE_FALSE(alwaysFalse());

	detail::ConstantInvokable<1337> always1337;
	REQUIRE(always1337() == 1337);
}

TEST_CASE("FlushPolicy should return results determined by predicate", "[Flush-Policy]")
{
	auto shallFilter = GENERATE(false, true);

	auto predicate = [shallFilter](const auto& rec, std::size_t byteCount)
	{
		return shallFilter;
	};

	FlushPolicy policy{ predicate };

	REQUIRE(policy(Record_t{}, 0) == shallFilter);
}

TEST_CASE("AlwaysFlushPolicy should always return true", "[Flush-Policy]")
{
	AlwaysFlushPolicy policy{};

	REQUIRE(policy(Record_t{}, 0));
	REQUIRE(policy(Record_t{}, 0));
}

TEST_CASE("makeMessageFlushPolicyFor function should create Policy watching on Record's message member", "[Flush-Policy]")
{
	Record_t record;
	auto policy = makeMessageFlushPolicyFor<Record_t>(Equals{ 1337 });

	REQUIRE_FALSE(std::invoke(policy, record, 0));

	record.setMessage(1337);

	REQUIRE(std::invoke(policy, record, 0));
}

TEST_CASE("makeSeverityFlushPolicyFor function should create Policy watching on Record's severity member", "[Flush-Policy]")
{
	Record_t record;
	auto policy = makeSeverityFlushPolicyFor<Record_t>(Equals{ 1337 });

	REQUIRE_FALSE(std::invoke(policy, record, 0));

	record.setSeverity(1337);

	REQUIRE(std::invoke(policy, record, 0));
}

TEST_CASE("makeChannelFlushPolicyFor function should create Policy watching on Record's channel member", "[Flush-Policy]")
{
	Record_t record;
	auto policy = makeChannelFlushPolicyFor<Record_t>(Equals{ 1337 });

	REQUIRE_FALSE(std::invoke(policy, record, 0));

	record.setChannel(1337);

	REQUIRE(std::invoke(policy, record, 0));
}

TEST_CASE("makeTimePointFlushPolicyFor function should create Policy watching on Record's timePoint member", "[Flush-Policy]")
{
	Record_t record;
	auto policy = makeTimePointFlushPolicyFor<Record_t>(Equals{ 1337 });

	REQUIRE_FALSE(std::invoke(policy, record, 0));

	record.setTimePoint(1337);

	REQUIRE(std::invoke(policy, record, 0));
}

TEST_CASE("ByteCountFlushPolicy should flush after a defined amount of data has been buffered", "[Flush-Policy]")
{
	const std::size_t threshold = 1024;
	ByteCountFlushPolicy policy{ threshold };

	SECTION("Accumulated amount of bytes less than threshold should yield negative results.")
	{
		REQUIRE_FALSE(policy(Record_t{}, 100));
		REQUIRE_FALSE(policy(Record_t{}, 100));
	}

	SECTION("Exceeding the threshold should yield once a positive result, followed up by negative results after flushed().")
	{
		REQUIRE(policy(Record_t{}, 1024));
		policy.flushed();
		REQUIRE_FALSE(policy(Record_t{}, 100));
	}
}

TEST_CASE("TimedFlushPolicy should flush after a defined duration has been exceeded", "[Flush-Policy]")
{
	const std::chrono::milliseconds threshold{ 10 };
	TimedFlushPolicy policy{ threshold * 2 };

	SECTION("Invoking during that duration should always yield a negative result.")
	{
		const auto begin = TimedFlushPolicy::Clock_t::now();
		while (threshold < TimedFlushPolicy::Clock_t::now() - begin)
		{
			REQUIRE_FALSE(policy(Record_t{}, 0));
		}
	}

	SECTION("Waiting until duration exceeded should yield a positive result, followed up by negative results after flushed().")
	{
		std::this_thread::sleep_for(threshold * 2);
		REQUIRE(policy(Record_t{}, 0));
		policy.flushed();
		REQUIRE_FALSE(policy(Record_t{}, 0));
	}
}
