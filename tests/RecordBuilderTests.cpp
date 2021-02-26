//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <optional>

#include "Simple-Log/Record.hpp"
#include "Simple-Log/RecordBuilder.hpp"

using namespace sl::log;

using Record_t = BaseRecord<int, int>;
using RecordBuilder_t = RecordBuilder<Record_t>;

namespace
{
	struct LogCallbackMoc
	{
		void operator ()(Record_t record) const
		{
			if (invoked)
			{
				*invoked = true;
			}

			if (targetRec)
			{
				*targetRec = std::move(record);
			}
		}

		bool* invoked = nullptr;
		Record_t* targetRec = nullptr;
	};
}

TEST_CASE("RecordBuilders should have a valid state after construction succeeded", "[RecordBuilder]")
{
	Record_t record;
	record.setSeverity(1337);
	RecordBuilder_t recBuilder{ record, LogCallbackMoc{} };

	REQUIRE(std::as_const(recBuilder).record().severity() == record.severity());
	REQUIRE(recBuilder.record().severity() == record.severity());
}

TEST_CASE("RecordBuilders should transfer its state to the move target", "[RecordBuilder]")
{
	Record_t record;
	record.setSeverity(1337);
	std::optional<RecordBuilder_t> targetRecBuilder;
	bool invoked = false;
	{
		RecordBuilder_t recBuilder{ record, LogCallbackMoc{ &invoked } };
		targetRecBuilder = std::move(recBuilder);
	}

	REQUIRE_FALSE(invoked);
	REQUIRE(targetRecBuilder->record().severity() == record.severity());

	targetRecBuilder.reset();
	REQUIRE(invoked);
}

TEST_CASE("RecordBuilder's operator << should append strings to the internal stream.", "[RecordBuilder]")
{
	Record_t targetRecord;
	std::optional<RecordBuilder_t> recBuilder{ std::in_place, Record_t{}, LogCallbackMoc{ .targetRec = &targetRecord } };
	*recBuilder << 1337;
	recBuilder.reset();

	REQUIRE(targetRecord.message() == "1337");
}

TEST_CASE("SetSev manipulator should manipulate Record's severity property.", "[RecordBuilder]")
{
	RecordBuilder_t recBuilder{ Record_t{}, LogCallbackMoc{} };
	recBuilder << SetSev{ 1337 };

	REQUIRE(recBuilder.record().severity() == 1337);
}

TEST_CASE("SetChan manipulator should manipulate Record's channel property.", "[RecordBuilder]")
{
	RecordBuilder_t recBuilder{ Record_t{}, LogCallbackMoc{} };
	recBuilder << SetChan{ 1337 };

	REQUIRE(recBuilder.record().channel() == 1337);
}
