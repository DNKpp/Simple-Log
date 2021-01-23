//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <iostream>

#include "Simple-Log/Core.hpp"
#include "Simple-Log/BasicSink.hpp"
#include "Simple-Log/FileSink.hpp"
#include "Simple-Log/Logger.hpp"
#include "Simple-Log/StringPattern.hpp"

using namespace sl::log;

inline Core core;

TEST_CASE(" ", "[Core]")
{

	auto& sink = core.makeSink<BasicSink>(std::cout);
	auto& fileSink = core.makeSink<FileSink>("test-%Y-%m-%d_%3N.log");
	fileSink.setRotationRule({ .fileSize = 0 });
	fileSink.setCleanupRule({.fileCount = 3});

	Logger log{ core, SeverityLevel::info };

	log << "Hello," << "World!";
	sink.setFormatter([](std::ostream& out, const Record& rec) { out << "yes" << rec.message; });
	sink.setFilter([](const Record& rec){ return rec.message != "Hello, World!"; });

	for (int i = 0; i < 10; ++i)
	{
		log << "log: " << i;
	}

	StringPattern pattern{ "hfkjdfk%m%N%3243Nfjlksdjfldsk%H-%M-%S%555JN" };

	std::this_thread::sleep_for(std::chrono::seconds{ 2 });
}
