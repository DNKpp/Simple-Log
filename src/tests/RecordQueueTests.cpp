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

SCENARIO("size/empty/push/take - non-blocking", "[RecordQueue]")
{
	RecordQueue<Record_t> queue;
	REQUIRE(std::empty(queue));
	REQUIRE(std::size(queue) == 0);

	WHEN("pushing new Records")
	{
		THEN("queue will grow")
		{
			auto records = generateRecordsWithIncrMsg(10);
			for (std::size_t i = 0; i < std::size(records); ++i)
			{
				queue.push(records[i]);
				REQUIRE(!std::empty(queue));
				auto expectedSize = i + 1;
				REQUIRE(std::size(queue) == expectedSize);
			}

			AND_WHEN("taking these values")
			{
				THEN("receiving them in insertion order")
				{
					for (std::size_t i = 0; i < std::size(records); ++i)
					{
						REQUIRE(!std::empty(queue));
						auto expectedSize = std::size(records) - i;
						REQUIRE(std::size(queue) == expectedSize);
						auto rec = queue.take();

						REQUIRE(rec);
						REQUIRE(rec->message() == records[i].message());
					}
				}
			}
		}
	}
}

SCENARIO("push/take - blocking", "[RecordQueue]")
{
	RecordQueue<Record_t> queue;
	REQUIRE(std::empty(queue));
	REQUIRE(std::size(queue) == 0);

	WHEN("requesting Records on empty queue")
	{
		THEN("will wait and break after some time")
		{
			std::chrono::milliseconds duration{ 100 };

			for (int i = 1; i < 5; ++i)
			{
				auto actualDuration = i * duration;
				auto begin = std::chrono::steady_clock::now();
				auto record = queue.take(actualDuration);

				REQUIRE(record == std::nullopt);

				auto end = std::chrono::steady_clock::now();
				auto diff = end - begin;
				REQUIRE(actualDuration <= diff);
			}
		}
	}

	WHEN("requesting Records on empty queue")
	{
		AND_WHEN("pushing value afterwards")
		{
			THEN("will wait and return record")
			{
				std::chrono::milliseconds waitDuration{ 100 };
				auto records = generateRecordsWithIncrMsg(10);
				RecordQueue<Record_t> queue;
				for (std::size_t i = 0; i < std::size(records); ++i)
				{
					std::atomic_bool finished{ false };
					auto future = std::async(
											std::launch::async,
											[&queue, &finished]()
											{
												auto rec = queue.take();
												finished = true;
												return rec;
											}
											);

					std::this_thread::sleep_for(waitDuration);
					REQUIRE(finished == false);

					queue.push(records[i]);

					auto futureState = future.wait_for(waitDuration);
					REQUIRE(futureState == std::future_status::ready);

					auto record = future.get();
					REQUIRE(record != std::nullopt);
					REQUIRE(record->message() == i);
				}
			}
		}
	}
}
