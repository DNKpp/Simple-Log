//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch.hpp"

#include <iostream>

#include "Simple-Log/Core.hpp"
#include "Simple-Log/BasicSink.hpp"

TEST_CASE(" ", "[Core]")
{
	using namespace sl::log;

	Core core;
	core.makeSink<BasicSink>(std::cout);

	std::this_thread::sleep_for(std::chrono::seconds{ 1 });

	core.log({ "Hello, World!" });
}
