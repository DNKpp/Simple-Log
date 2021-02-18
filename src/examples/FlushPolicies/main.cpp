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
	auto core = std::make_unique<logging::Core_t>();

	// create Sink in disabled state, thus we are able to configure it before it does handle any records
	// will be automatically enabled when current scoped is left
	auto consoleSink = core->makeDisabledSink<logging::BasicSink_t>(std::cout);

	consoleSink->setFlushPolicy(
		sl::log::makeSeverityFlushPolicyFor<logging::Record_t>(sl::log::Greater{ logging::SeverityLevel::error })		// performs flush if Record with error level has been handled.
	);

	// note: this will override the previous policy
	consoleSink->setFlushPolicy(
		sl::log::ByteCountFlushPolicy{ 100 }	// performs flush when the threshold of 100 bytes is exceeded
	);
	return core;
}

inline std::unique_ptr<logging::Core_t> gLoggingCore{ makeLoggingCore() };
inline logging::Logger_t gLog{ *gLoggingCore, logging::SeverityLevel::info };

int main()
{
	gLog() << "Hello, World!";																					// not flushed

	gLog() << "Mighty debug message";																			// not flushed

	gLog() << "Print my important hint!";																		// flushes here

	gLog() << "This" << " will" << " create" << " a" << " concatenated" << " warning" << " message";			// not flushed

	gLog() << "Back to info";																					// flushes again
}
