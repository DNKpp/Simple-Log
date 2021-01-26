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
	/** \addtogroup record
	 * @{
	 */

	/**
	 * \brief A simple severity level enum type
	 * \details This type serves as a default solution for severity levels but doesn't have to be used. You are free to declare your custom enum type or anything more exotic you can imagine.
	 */
	enum class SeverityLevel
	{
		info,
		debug,
		hint,
		warning,
		error,
		fatal
	};

	/**
	 * \brief Operator << overload for the default severity type
	 * \details This is a simple overload for the library provided SeverityLevel, which will be used in the default formatter if such a type is detected. It simply translates the enum value name to a string representation.
	 * \param out the stream object
	 * \param lvl the severity level
	 * \return Returns the parameter out as reference
	 */
	inline std::ostream& operator <<(std::ostream& out, SeverityLevel lvl)
	{
		constexpr const char* str[] = { "info", "debug", "hint", "warning", "error", "fatal" };
		out << str[static_cast<std::size_t>(lvl)];
		return out;
	}

	/**
	 * \brief A collection of logging related information
	 * \details This class serves as a simple collection of all the gathered information during a logging action.
	 */
	class Record
	{
	public:
		/**
		 * \brief Stores the logged message.
		 */
		std::string message;

		/**
		 * \brief Stores the time point when the Record was made.
		 */
		std::chrono::system_clock::time_point time;
#ifdef __cpp_lib_source_location
		std::source_location sourceLocation;
#endif

		/**
		 * \brief Stores the Record s severity level. Can be of any type.
		 */
		std::any severity;
		/**
		 * \brief Stores the Record s related channel. Can be of any type.
		 */
		std::any channel;
		/**
		 * \brief Stores additional user data. Can be of any type.
		 */
		std::any userData;
	};

	/** @}*/
}

#endif
