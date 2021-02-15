//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <Simple-Log/PresetTypes.hpp>

#include <iostream>
#include <memory>

namespace logging = sl::log::pre;

inline std::unique_ptr<logging::Core_t> makeLoggingCore()
{
	/* an sl::log::Core is non-copy and non-movable, thus we will store it in an unique_ptr, so we
	 * can safely move it into our global.*/
	auto core = std::make_unique<logging::Core_t>();

	/* register a BasicSink and link it to the std::cout stream. This will simply print all incoming
	messages onto the console.*/
	core->makeSink<logging::BasicSink_t>(std::cout);
	return core;
}

/* For conveniences we will simply store the core and our default logger as a global. Feel free to do it
otherwise. Just make sure the Core instance doesn't get destructed before all related Logger instances.*/
inline std::unique_ptr<logging::Core_t> gLoggingCore{ makeLoggingCore() };
inline logging::Logger_t gLog{ *gLoggingCore, logging::SeverityLevel::info };

int main()
{
	gLog() << "Hello, World!";	// This will print this message with the "info" severity
	// creates a record with severity level "debug".
	//gLog() << "Mighty debug message" << sl::log::SetSev{ sl::log::def::SeverityLevel::debug };
}

/*Core will make sure, that all pending Records will be processed before it gets destructed.*/
