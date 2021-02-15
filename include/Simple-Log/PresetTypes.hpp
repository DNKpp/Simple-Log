//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_PRESET_TYPES_HPP
#define SL_LOG_PRESET_TYPES_HPP

#pragma once

#include "Simple-Log.hpp"

namespace sl::log::pre
{
	/**
	 * \brief A simple severity level enum type
	 * \details This type serves as a default solution for severity levels but doesn't have to be used. You are free to declare your custom enum type or anything more exotic you can imagine.
	 */
	enum class SeverityLevel
	{
		debug,
		info,
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
		constexpr const char* str[] = { "debug", "info", "hint", "warning", "error", "fatal" };
		out << str[static_cast<std::size_t>(lvl)];
		return out;
	}
	
	using Record_t = BaseRecord<SeverityLevel, std::string>;
	using Core_t = Core<Record_t>;
	using Logger_t = Logger<Record_t>;
	using ISink_t = ISink<Record_t>;
	using BasicSink_t = BasicSink<Record_t>;
	using FileSink_t = FileSink<Record_t>;
}

#endif
