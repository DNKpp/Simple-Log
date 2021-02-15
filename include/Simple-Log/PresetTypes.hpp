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
	/** \addtogroup PreparedTypes Prepared Types
	 * @{
	 * \details Each type serves as a default solution for simple logging, but users are free to dig deeper and define their custom types and rules. In this namespace are mainly type aliases
	 * which are a good starting point for getting used to the library.
	 */

	/**
	 * \brief A simple severity level enum type
	 * \details Each value reflects a distinguishable severity level on which formatter and filter of Sink objects can react.
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
	 * \brief Operator << overload for SeverityLevel type
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

	struct SetSev
	{
		using SetSev_t = log::SetSev<SeverityLevel>;
		
		constexpr static SetSev_t debug{ SeverityLevel::debug };
		constexpr static SetSev_t info{ SeverityLevel::info };
		constexpr static SetSev_t hint{ SeverityLevel::hint };
		constexpr static SetSev_t warning{ SeverityLevel::warning };
		constexpr static SetSev_t error{ SeverityLevel::error };
		constexpr static SetSev_t fatal{ SeverityLevel::fatal };
	};

	/**
	 * \brief Prepared Record type
	 * \details Record_t is an type alias for the BaseRecord and uses SeverityLevel as severity level type and std::string as channel identifier.
	 */
	using Record_t = BaseRecord<SeverityLevel, std::string>;
	/**
	 * \brief Type alias for log::Core which uses prep::Record_t as Record type
	 */
	using Core_t = Core<Record_t>;
	/**
	 * \brief Type alias for log::Logger which uses prep::Record_t as Record type
	 */
	using Logger_t = Logger<Record_t>;
	/**
	 * \brief Type alias for log::ISink which uses prep::Record_t as Record type
	 */
	using ISink_t = ISink<Record_t>;
	/**
	 * \brief Type alias for log::BasicSink which uses prep::Record_t as Record type
	 */
	using BasicSink_t = BasicSink<Record_t>;
	/**
	 * \brief Type alias for log::FileSink which uses prep::Record_t as Record type
	 */
	using FileSink_t = FileSink<Record_t>;

	/** @}*/
}

#endif
