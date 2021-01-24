//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_RECORD_HPP
#define SL_LOG_RECORD_HPP

#pragma once

#include <any>
#include <chrono>
#include <string>
#include <version>

#ifdef __cpp_lib_source_location
	#include <source_location>
#endif

namespace sl::log
{
	enum class SeverityLevel
	{
		info,
		debug,
		hint,
		warning,
		error,
		fatal
	};

	std::ostream& operator <<(std::ostream& out, SeverityLevel lvl)
	{
		constexpr const char* str[] = { "info", "debug", "hint", "warning", "error", "fatal" };
		out << str[static_cast<std::size_t>(lvl)];
		return out;
	}
	
	class Record
	{
	public:
		std::string message;
		std::chrono::system_clock::time_point time;
#ifdef __cpp_lib_source_location
		std::source_location sourceLocation;
#endif

		std::any severity;
		std::any channel;
		std::any userData;
	};
}

#endif
