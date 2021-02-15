//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <concepts>
#include <iostream>
#include "Simple-Log/PresetTypes.hpp"

using namespace sl::log;

using Record_t = BaseRecord<pre::SeverityLevel, std::string, std::string, std::chrono::system_clock::time_point>;
inline Core<Record_t> core;

enum class Channel
{
	test
};

struct Formatter
{
	template <Record TRecord>
	void operator ()(std::ostream& out, const TRecord& rec)
	{
	}
};

using Sev = pre::SetSev;

TEST_CASE(" ", "[Core]")
{
	auto& sink = core.makeSink<BasicSink<Record_t>>(std::cout);
	auto& fileSink = core.makeSink<FileSink<Record_t>>("test-%Y-%m-%d_%3N.log");
	fileSink.setRotationRule({ .fileSize = 10 * 1024 * 1024 });
	fileSink.setCleanupRule({ .fileCount = 20 });

	fileSink.setFilter(
		FilterAllOf
		{
			ProjectionFilter{ &Record_t::severity, pred::LessToConstant{ pre::SeverityLevel::info }},
			ProjectionFilter{ &Record_t::channel, pred::NotEqualsToConstant{ "test" } },
			makeChannelFilterFor<Record_t>(pred::NotEqualsToConstant{ "test" })
		}
	);

	//auto f{ makeChannelFilter<Channel>(pred::LessToConstant{ Channel::test }) };
	//FilterConjunction c{ f, f };
	//fileSink.setFilter(c);
	//fileSink.setFilter(Filter{ &Record::time, EqualsToConstant{ std::chrono::steady_clock::now() }});
	
	Logger<Record_t> log{ core, pre::SeverityLevel::info };

	log() << SetSev(pre::SeverityLevel::debug) << "Hello," << SetChan("test") << "World!";
	log() << "Hello, Zhavok!" << Sev::error;

	//sink.setFormatter([](std::ostream& out, const Record& rec) { out << "yes" << rec.message; });
	//sink.setFilter([](const Record& rec){ return rec.message != "Hello, World!"; });

	for (int i = 0; i < 1000; ++i)
	{
		log() << "log: " << i;
	}

	std::this_thread::sleep_for(std::chrono::seconds{ 2 });
}
