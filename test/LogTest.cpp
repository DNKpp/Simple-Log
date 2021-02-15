//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <iostream>

#include "Simple-Log/PresetTypes.hpp"

using namespace sl::log;

inline pre::Core_t core;

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

TEST_CASE(" ", "[Core]")
{

	auto& sink = core.makeSink<pre::BasicSink_t>(std::cout);
	//auto& fileSink = core.makeSink<FileSink>("test-%Y-%m-%d_%3N.log");
	//fileSink.setRotationRule({ .fileSize = 10 * 1024 * 1024 });
	//fileSink.setCleanupRule({.fileCount = 20});
	//fileSink.setFilter([](const Record& rec)
	//{
	//	if (auto channel = std::any_cast<Channel>(&rec.channel); channel && *channel == Channel::test)
	//		return false;
	//	return true;
	//});

	//fileSink.setFormatter(Formatter{});

	pre::Logger_t log{ core, pre::SeverityLevel::info };

	log() << SetSev(pre::SeverityLevel::debug) << "Hello," << SetChan("test") << "World!";
	log() << "Hello, Zhavok!";
	
	//sink.setFormatter([](std::ostream& out, const Record& rec) { out << "yes" << rec.message; });
	//sink.setFilter([](const Record& rec){ return rec.message != "Hello, World!"; });

	for (int i = 0; i < 1000; ++i)
	{
		log() << "log: " << i;
	}

	std::this_thread::sleep_for(std::chrono::seconds{ 2 });
}
