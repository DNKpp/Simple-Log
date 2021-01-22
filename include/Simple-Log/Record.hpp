//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_RECORD_HPP
#define SL_LOG_RECORD_HPP

#pragma once

#include <any>
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
		hint,
		warning,
		error,
		fatal
	};

	class Record
	{
	public:		
		std::string message;

#ifdef __cpp_lib_source_location
		std::source_location sourceLocation;
#endif

		std::any severity;
		std::any channel;
		std::any userData;
	};
}

#endif
