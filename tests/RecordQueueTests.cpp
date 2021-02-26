//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch2/catch.hpp"

#include "Simple-Log/Record.hpp"
#include "Simple-Log/RecordQueue.hpp"

#include <future>

using namespace sl::log;

using Record_t = BaseRecord<int, int, int>;

static std::vector<Record_t> generateRecordsWithIncrMsg(std::size_t count)
{
	std::vector<Record_t> records;
	records.reserve(count);
	std::ranges::generate_n(
							std::back_inserter(records),
							count,
							[i = 0]() mutable
							{
								Record_t record;
								record.setMessage(i++);
								return record;
							}
							);
	return records;
}

SCENARIO("RecordQueue's empty and size should yield expected results.", "[RecordQueue]")
{
	RecordQueue<Record_t> queue;

	GIVEN("an empty queue")
	{
		WHEN("calling empty()")
		THEN("should yield true")
		{
			REQUIRE(queue.empty());
		}

		WHEN("calling size()")
		THEN("should yield 0")
		{
			REQUIRE(queue.size() == 0); // NOLINT(readability-container-size-empty)
		}
	}

	GIVEN("a non-empty queue")
	{
		queue.push({});
		WHEN("calling empty()")
		THEN("should yield false")
		{
			REQUIRE_FALSE(queue.empty());
		}

		WHEN("calling size()")
		THEN("should yield expected size")
		{
			REQUIRE(queue.size() == 1);
		}
	}
}

SCENARIO("RecordQueue should provide values in insertion-order (FIFO).", "[RecordQueue]")
{
	RecordQueue<Record_t> queue;

	GIVEN("an empty queue")
	WHEN("pushing new Records")
	THEN("queue will grow")
	{
		auto records = generateRecordsWithIncrMsg(10);
		for (std::size_t i = 0; i < std::size(records); ++i)
		{
			queue.push(records[i]);
		}

		REQUIRE(std::size(queue) == std::size(records));

		AND_WHEN("taking these Records")
		THEN("caller will receive them in insertion-order")
		{
			for (auto& record : records)
			{
				auto takenRecord = queue.take();
				REQUIRE(takenRecord);
				REQUIRE(takenRecord->message() == record.message());
			}

			REQUIRE(std::empty(queue));
		}
	}
}

TEMPLATE_PRODUCT_TEST_CASE(
							"RecordQueue's take should behave as expected in blocking and non-blocking cases.",
							"[RecordQueue]",
							RecordQueue,
							(
								Record_t,
								(Record_t, detail::BlockingPushTake<Record_t, 10>)
							)
						)
{
	TestType queue;
	REQUIRE(std::empty(queue));
	REQUIRE(std::size(queue) == 0);

	GIVEN("an empty queue")
	WHEN("requesting Records on empty queue")
	THEN("will block and break after specified duration")
	{
		std::chrono::milliseconds duration{ 100 };

		auto begin = std::chrono::steady_clock::now();
		auto record = queue.take(duration);
		auto end = std::chrono::steady_clock::now();
		auto diff = end - begin;

		REQUIRE(record == std::nullopt);
		REQUIRE(duration <= diff);
	}

	GIVEN("an empty queue")
	WHEN("requesting Records")
	THEN("will wait until Record is inserted and will return an valid Record")
	{
		std::chrono::milliseconds waitDuration{ 100 };
		std::atomic_bool finished{ false };
		auto future = std::async(
								std::launch::async,
								[&queue, &finished]()
								{
									const auto rec = queue.take();
									finished = true;
									return rec;
								}
								);

		std::this_thread::sleep_for(waitDuration);
		REQUIRE(finished == false);

		AND_WHEN("pushing Records while blocking")
		THEN("will break and return that Record")
		{
			Record_t inRecord;
			inRecord.setMessage(1337);
			queue.push(inRecord);
			auto futureState = future.wait_for(waitDuration);
			auto record = future.get();

			REQUIRE(futureState == std::future_status::ready);
			REQUIRE(record != std::nullopt);
			REQUIRE(record->message() == inRecord.message());
		}
	}
}

SCENARIO("RecordQueue's push should behave as expected in blocking and non-blocking cases.", "[RecordQueue]")
{
	RecordQueue<Record_t, detail::BlockingPushTake<Record_t, 1>> queue;
	REQUIRE(std::empty(queue));
	REQUIRE(std::size(queue) == 0);

	GIVEN("an empty queue")
	WHEN("pushing a Record")
	THEN("will neither block nor fail")
	{
		queue.push({});
		REQUIRE_FALSE(std::empty(queue));
	}

	GIVEN("a full queue")
	WHEN("pushing a Record")
	THEN("RecordQueue will block")
	{
		queue.push({});
		std::chrono::milliseconds waitDuration{ 100 };
		std::atomic_bool finished{ false };
		auto future = std::async(
								std::launch::async,
								[&queue, &finished]()
								{
									queue.push({});
									finished = true;
								}
								);

		std::this_thread::sleep_for(waitDuration);
		REQUIRE(finished == false);

		AND_WHEN("taking a Record while blocking")
		THEN("will break")
		{
			auto record = queue.take();
			auto futureState = future.wait_for(waitDuration);

			REQUIRE(futureState == std::future_status::ready);
		}
	}
}

TEMPLATE_PRODUCT_TEST_CASE(
							"RecordQueue's push should discard new Records if a \"discard at push\"-Strategy is applied.",
							"[RecordQueue]",
							RecordQueue,
							((Record_t, detail::DiscardedPushBlockingTake<Record_t, 1>))
						)
{
	TestType queue;
	REQUIRE(std::empty(queue));
	REQUIRE(std::size(queue) == 0);

	GIVEN("an empty queue")
	WHEN("pushing a Record")
	THEN("will neither block nor fail")
	{
		queue.push({});
		REQUIRE_FALSE(std::empty(queue));
	}

	GIVEN("a full queue")
	WHEN("pushing a Record")
	THEN("RecordQueue will block")
	{
		queue.push({});
		REQUIRE(std::size(queue) == 1);
		queue.push({});
		REQUIRE(std::size(queue) == 1);
	}
}
