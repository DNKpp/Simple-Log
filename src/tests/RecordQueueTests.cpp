//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch2/catch.hpp"

#include "Simple-Log/Record.hpp"
#include "Simple-Log/RecordQueue.hpp"

using namespace sl::log;

SCENARIO("size/empty/push/take - non-blocking", "[RecordQueue]")
{
	using Record_t = BaseRecord<int, int, int>;

	RecordQueue<Record_t> queue;
	REQUIRE(std::empty(queue));
	REQUIRE(std::size(queue) == 0);

	WHEN("pushing new Records")
	{
		std::vector<Record_t> records;
		std::ranges::generate_n(
								std::back_inserter(records),
								10,
								[i = 0]() mutable
								{
									Record_t record;
									record.setMessage(i);
									return record;
								}
								);
		
		THEN("queue will grow")
		{
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
						REQUIRE(std::empty(queue));
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
