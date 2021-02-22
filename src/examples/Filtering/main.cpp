//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <Simple-Log/PresetTypes.hpp>

// contains some helpful filter utility
#include <Simple-Log/Filters.hpp>

#include <iostream>
#include <memory>

namespace logging = sl::log::pre;

inline std::unique_ptr<logging::Core_t> makeLoggingCore()
{
	auto core = std::make_unique<logging::Core_t>();

	// makeSink returns a reference to the created Sink.
	auto& consoleSink = core->makeSink<logging::BasicSink_t>(std::cout);
	consoleSink.setFilter(
		// setup a simple filter
		sl::log::makeSeverityFilterFor<logging::Record_t>(sl::log::GreaterEquals{ logging::SeverityLevel::warning })
	);

	auto& welcomeChannelSink = core->makeSink<logging::BasicSink_t>(std::cout);
	welcomeChannelSink.setFilter(
		// This will simply combine any filter via AND. There exist also FilterAnyOf or FilterNoneOf utilities. You could even chain them together! Be creative ;)
		sl::log::FilterAllOf
		{
			sl::log::makeChannelFilterFor<logging::Record_t>(sl::log::Equals{ "Welcome" }),	// use factory make...FilterFor functions
			[](const auto& rec) { return rec.severity() != logging::SeverityLevel::debug; }		// or even lambdas
		}
	);
	return core;
}

inline auto gLoggingCore{ makeLoggingCore() };
inline auto gLog{ sl::log::makeLogger<logging::Logger_t>(*gLoggingCore, logging::SeverityLevel::info) };

int main()
{
	gLog() << sl::log::SetChan{ "Welcome" } << "Hello, World!";																			// Will be handled by welcomeChannelSink

	gLog() << sl::log::SetChan{ "Welcome" } << logging::SetSev::debug << "Mighty debug message";										// Will be skipped due to filter settings

	gLog() << "Print my important hint!" << logging::SetSev::hint;																			// Will be skipped due to filter settings

	gLog() << "This" << " will " << logging::SetSev::warning << " create " << " a " << " concatenated " << " warning " << " message";		// Will be handled by consoleSink

	gLog() << "Back to info";																												// Will be skipped due to filter settings
}

/*
 * The above code will for example generate this output:
 * 16:11:31.271 >>> info:: Hello, World!
 * 16:11:31.271 >>> warning:: This will  create  a  concatenated  warning  message
 *
 * All other generated Records violate any of our defined filters, thus won't be handled.
 */
