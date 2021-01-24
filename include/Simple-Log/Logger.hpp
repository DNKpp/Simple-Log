//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_LOGGER_HPP
#define SL_LOG_LOGGER_HPP

#pragma once

#include <any>
#include <cassert>
#include <sstream>

#include "Core.hpp"
#include "Record.hpp"

namespace sl::log
{
	struct SetSeverity
	{
		std::any severity;
	};
	using SetSev = SetSeverity;

	struct SetChannel
	{
		std::any channel;
	};
	using SetChan = SetChannel;

	struct SetUserData
	{
		std::any userData;
	};
	using SetData = SetUserData;
	
	class RecordBuilder
	{
	public:
		using LogCallback_t = std::function<void(Record)>;

		explicit RecordBuilder(LogCallback_t cb) noexcept :
			m_LogCallback{ std::move(cb) }
		{
		}

		~RecordBuilder() noexcept
		{
			if (m_LogCallback)
			{
				try
				{
					m_Record.message = std::move(m_Stream).str();
					m_LogCallback(std::move(m_Record));
				}
				catch (...)
				{
				}
			}
		}

		RecordBuilder(RecordBuilder&& other) noexcept
		{
			*this = std::move(other);
		}

		RecordBuilder& operator =(RecordBuilder&& other) noexcept
		{
			using std::swap;
			swap(m_Record, other.m_Record);
			swap(m_Stream, other.m_Stream);
			m_LogCallback = std::exchange(other.m_LogCallback, nullptr);
			return *this;
		}

		RecordBuilder(const RecordBuilder&) = delete;
		RecordBuilder& operator =(const RecordBuilder&) = delete;

		[[nodiscard]] Record& record() noexcept
		{
			return m_Record;
		}

		[[nodiscard]] const Record& record() const noexcept
		{
			return m_Record;
		}

		template <class T>
		requires requires (T&& data)
		{
			{ std::declval<std::ostringstream>() << std::forward<T>(data) };
		}
		RecordBuilder& operator <<(T&& data)
		{
			m_Stream << std::forward<T>(data);
			return *this;
		}

		RecordBuilder& operator <<(SetSeverity setSev) noexcept
		{
			m_Record.severity = std::move(setSev.severity);
			return *this;
		}

		RecordBuilder& operator <<(SetChannel setChannel) noexcept
		{
			m_Record.channel = std::move(setChannel.channel);
			return *this;
		}

		RecordBuilder& operator <<(SetUserData setUserData) noexcept
		{
			m_Record.channel = std::move(setUserData.userData);
			return *this;
		}

	private:
		Record m_Record;
		std::ostringstream m_Stream;
		std::function<void(Record)> m_LogCallback;
	};

	class Logger
	{
	public:
		Logger(
			Core& core,
			SeverityLevel defaultSeverityLvl,
			std::any defaultChannel = {},
			std::any defaultUserData = {}
		) noexcept :
			m_Core{ &core },
			m_DefaultSeverityLvl{ defaultSeverityLvl },
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
		SeverityLevel m_DefaultSeverityLvl;
		std::any m_DefaultChannel;
		std::any m_DefaultUserData;
	};
}

#endif
