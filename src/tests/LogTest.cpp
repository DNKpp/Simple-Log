//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch2/catch.hpp"

#include <concepts>
#include <iostream>
#include "Simple-Log/Simple-Log.hpp"

using namespace sl::log;

inline Core core;

enum class Channel
{
	test
};

TEST_CASE(" ", "[Core]")
{
	auto& sink = core.makeSink<BasicSink>(std::cout);
	auto& fileSink = core.makeSink<FileSink>("test-%Y-%m-%d_%3N.log");
	fileSink.setRotationRule({ .fileSize = 10 * 1024 * 1024 });
	fileSink.setCleanupRule({ .fileCount = 20 });

	auto f{ makeChannelFilter<Channel>(pred::LessToConstant{ Channel::test }) };
	FilterConjunction c{ f, f };
	fileSink.setFilter(c);
	//fileSink.setFilter(Filter{ &Record::time, EqualsToConstant{ std::chrono::steady_clock::now() }});

	Logger log{ core, SeverityLevel::info };

	log() << SetSeverity(SeverityLevel::debug) << "Hello," << SetChannel(Channel::test) << "World!";
	log() << "Hello, Zhavok!";

	//sink.setFormatter([](std::ostream& out, const Record& rec) { out << "yes" << rec.message; });
	//sink.setFilter([](const Record& rec){ return rec.message != "Hello, World!"; });

	for (int i = 0; i < 1000; ++i)
	{
		log() << "log: " << i;
	}

	std::this_thread::sleep_for(std::chrono::seconds{ 2 });
}
