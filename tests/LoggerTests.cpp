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
							"[BaseLogger]",
							BaseLogger,
							(
								(BaseRecord<std::string, int>),
								(BaseRecord<int, std::string>)
							)
						)
{
	using Logger_t = TestType;
	using Record_t = typename Logger_t::Record_t;

	Logger_t logger{ LogCallbackMoc{} };
	REQUIRE(logger.defaultSeverity() == typename Record_t::SeverityLevel_t{});
	REQUIRE(logger.defaultChannel() == typename Record_t::Channel_t{});
}

TEMPLATE_TEST_CASE_SIG(
						"BaseLoggers should have a valid state after construction succeeded",
						"[BaseLogger]",
						((auto sev, auto chan), sev, chan),
						(1, 2)
					)
{
	using Record_t = BaseRecord<decltype(sev), decltype(chan)>;
	using Logger_t = BaseLogger<Record_t>;

	Logger_t logger{ LogCallbackMoc{}, sev, chan };

	REQUIRE(logger.defaultSeverity() == sev);
	REQUIRE(logger.defaultChannel() == chan);
}

TEMPLATE_TEST_CASE_SIG(
						"BaseLoggers getter should yield equal results after setter were used.",
						"[BaseLogger]",
						((auto sev, auto chan), sev, chan),
						(1, 2)
					)
{
	using Record_t = BaseRecord<decltype(sev), decltype(chan)>;
	using Logger_t = BaseLogger<Record_t>;

	Logger_t logger{ LogCallbackMoc{} };
	logger.setDefaultSeverity(sev);
	logger.setDefaultChannel(chan);

	REQUIRE(logger.defaultSeverity() == sev);
	REQUIRE(logger.defaultChannel() == chan);
}

TEST_CASE("BaseLogger should invoke its callback after their created RecordBuilder gets destroyed.", "[BaseLogger]")
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
