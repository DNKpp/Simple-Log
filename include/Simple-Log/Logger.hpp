//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_LOGGER_HPP
#define SL_LOG_LOGGER_HPP

#pragma once

#include <any>
#include <cassert>
#include <version>

#ifdef __cpp_lib_source_location
#include <source_location>
#endif

#include "Core.hpp"
#include "Record.hpp"
#include "RecordBuilder.hpp"

namespace sl::log
{
	class Logger
	{
	public:
		Logger(
			Core& core,
			std::any defaultSeverityLvl = {},
			std::any defaultChannel = {},
			std::any defaultUserData = {}
		) noexcept :
			m_Core{ &core },
			m_DefaultSeverityLvl{ std::move(defaultSeverityLvl) },
			m_DefaultChannel{ std::move(defaultChannel) },
			m_DefaultUserData{ std::move(defaultUserData) }
		{
		}

		~Logger() noexcept = default;

		Logger(const Logger&) noexcept = default;
		Logger& operator =(const Logger&) noexcept = default;
		Logger(Logger&&) noexcept = default;
		Logger& operator =(Logger&&) noexcept = default;

		// ReSharper disable once CppMemberFunctionMayBeConst
		// Logger should semantically not be const, when writing to the logs
		void log(Record rec)
		{
			assert(m_Core);
			m_Core->log(std::move(rec));
		}

#ifdef __cpp_lib_source_location
		RecordBuilder operator ()(const std::source_location& srcLoc = std::source_location::current())
#else
		RecordBuilder operator ()()
#endif
		{
			RecordBuilder builder{ [&logger = *this](Record rec) { logger.log(std::move(rec)); } };
			builder.record().severity = m_DefaultSeverityLvl;
			builder.record().channel = m_DefaultChannel;
			builder.record().userData = m_DefaultUserData;

#ifdef __cpp_lib_source_location
			builder.record().sourceLocation = srcLoc;
#endif

			return builder;
		}

	private:
		Core* m_Core = nullptr;
		std::any m_DefaultSeverityLvl;
		std::any m_DefaultChannel;
		std::any m_DefaultUserData;
	};
}

#endif
