//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <Simple-Log/Core.hpp>
#include <Simple-Log/BasicSink.hpp>
#include <Simple-Log/Logger.hpp>

#include <iostream>
#include <memory>

inline std::unique_ptr<sl::log::Core> makeLoggingCore()
{
	/* an sl::log::Core is non-copy and non-movable, thus we will store it in an unique_ptr, so we
	 * can safely move it into our global.*/
	auto core = std::make_unique<sl::log::Core>();

	/* register a BasicSink and link it to the std::cout stream. This will simply print all incoming
	messages onto the console.*/
	core->makeSink<sl::log::BasicSink>(std::cout);
	return core;
}

/* For conveniences we will simply store the core and our default logger as a global. Feel free to do it
otherwise. Just make sure the Core instance doesn't get destructed before all related Logger instances.*/
inline std::unique_ptr<sl::log::Core> gLoggingCore{ makeLoggingCore() };
inline sl::log::Logger gLog{ *gLoggingCore, sl::log::SeverityLevel::info };

int main()
{
	gLog() << "Hello, World!";	// This will print this message with the "info" severity
	// creates a record with severity level "debug".
	gLog() << "Mighty debug message" << sl::log::SetSev{ sl::log::SeverityLevel::debug };
}

/*Core will make sure, that all pending Records will be processed before it gets destructed.*/
