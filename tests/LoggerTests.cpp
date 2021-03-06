//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include "Simple-Log/Logger.hpp"
#include "Simple-Log/Record.hpp"

using namespace sl::log;

namespace
{
	struct LogCallbackMoc
	{
		template <class TRecord>
		void operator ()(TRecord) const
		{
			if (invoked)
			{
				*invoked = true;
			}
		}

		bool* invoked = nullptr;
	};
}

TEMPLATE_PRODUCT_TEST_CASE(
							"BaseLoggers should have a valid state after default construction succeeded",
							"[BaseLogger][Logger]",
							BaseLogger,
							(
								(BaseRecord<std::string, int>),
								(BaseRecord<int, std::string>)
							)
						)
{
	using Logger_t = TestType;
	using Record_t = LoggerRecord_t<Logger_t>;

	Logger_t logger{ LogCallbackMoc{} };
	REQUIRE(logger.defaultSeverity() == typename Record_t::SeverityLevel_t{});
	REQUIRE(logger.defaultChannel() == typename Record_t::Channel_t{});
}

TEMPLATE_TEST_CASE_SIG(
						"BaseLoggers should have a valid state after construction succeeded",
						"[BaseLogger][Logger]",
						((auto VSev, auto VChan), VSev, VChan),
						(1, 2)
					)
{
	using Record_t = BaseRecord<decltype(VSev), decltype(VChan)>;
	using Logger_t = BaseLogger<Record_t>;

	Logger_t logger{ LogCallbackMoc{}, VSev, VChan };

	REQUIRE(logger.defaultSeverity() == VSev);
	REQUIRE(logger.defaultChannel() == VChan);
}

TEMPLATE_TEST_CASE_SIG(
						"BaseLoggers getter should yield equal results after setter were used.",
						"[BaseLogger][Logger]",
						((auto VSev, auto VChan), VSev, VChan),
						(1, 2)
					)
{
	using Record_t = BaseRecord<decltype(VSev), decltype(VChan)>;
	using Logger_t = BaseLogger<Record_t>;

	Logger_t logger{ LogCallbackMoc{} };
	logger.setDefaultSeverity(VSev);
	logger.setDefaultChannel(VChan);

	REQUIRE(logger.defaultSeverity() == VSev);
	REQUIRE(logger.defaultChannel() == VChan);
}

TEST_CASE("BaseLogger should invoke its callback after their created RecordBuilder gets destroyed.", "[BaseLogger][Logger]")
{
	using Record_t = BaseRecord<int, int>;
	using Logger_t = BaseLogger<Record_t>;

	bool invoked = false;
	Logger_t logger{ LogCallbackMoc{ &invoked } };

	{
		auto recBuilder = logger();
		REQUIRE_FALSE(invoked);
	}
	REQUIRE(invoked);
}

namespace custom
{
	struct Logger
	{
		using ORecord_t = BaseRecord<int, int>;

		using RecordBuilder_t = RecordBuilder<ORecord_t>;

		template <class TCallback>
		explicit Logger(TCallback cb)
		{
		}

		RecordBuilder_t operator()()
		{
			return RecordBuilder_t{
				{},
				[](ORecord_t&&)
				{
				}
			};
		}
	};
}

template <>
struct sl::log::LoggerTypedefs<custom::Logger>
{
	using Record_t = custom::Logger::ORecord_t;
};

TEST_CASE("Custom Logger type using the abstractions should compile successfully.", "[Logger]")
{
	using Logger_t = custom::Logger;
	using Typedefs_t = LoggerTypedefs<Logger_t>;
	using Core_t = Core<LoggerRecord_t<Logger_t>>;

	REQUIRE(std::is_same_v<typename Typedefs_t::Record_t, Logger_t::ORecord_t>);
	REQUIRE(std::is_same_v<LoggerRecord_t<Logger_t>, Logger_t::ORecord_t>);

	Core_t core;
	auto logger = makeLogger<Logger_t>(core);
	logger();
}
