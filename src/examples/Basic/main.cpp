//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

// This is the header of preset types, thus you'll have a easy start getting used to the library.
#include <Simple-Log/PresetTypes.hpp>

#include <iostream>
#include <memory>

/* let's use a namepsace alias for convenience; be aware: if you use cmath, some stl implementations will bloat your global namespace with a log function declaration (c-relict).
Thus to make it compatible with all compilers, I'll use logging as alias instead.

All preset type alias are located in the sl::log::pre namespace, thus they do not interfere with the actual library if you don't want them to.
It is no good style just importing everything into your namespace. Just create an namespace alias like so. This way it's very easy to make it less verbose for you.
*/
namespace logging = sl::log::pre;

// We're using a factory function here, but this isn't necessary. You could also create a plain Core instance and set it up later in main
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
	// override default logger settings at the beginning
	gLog() << logging::SetSev::debug << "Mighty debug message";
	// or at the end
	gLog() << "Print my important hint!" << logging::SetSev::hint;
	// or in between of messages
	gLog() << "This" << " will " << logging::SetSev::warning << " create " << " a " << " concatenated " << " warning " << " message";
	// and using default setup again
	gLog() << "Back to info";
}

/*Core will make sure, that all pending Records will be processed before it gets destructed.*/
/*
 * The above code will for example generate this output:
 *	18:49:32.047 >>> info:: Hello, World!
 *	18:49:32.047 >>> debug:: Mighty debug message
 *	18:49:32.047 >>> hint:: Print my important hint!
 *	18:49:32.047 >>> warning:: This will  create  a  concatenated  warning  message
 *	18:49:32.047 >>> info:: Back to info
 *
 * Keep in mind, you are completely free how you are going to format your message. This is just the default one.
 */
