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
	class Logger;

	class RecordBuilder
	{
	public:
#ifdef __cpp_lib_source_location
		RecordBuilder(const std::source_location& srcLoc = std::source_location::current()) noexcept
			m_Record{ .sourceLocation = srcLoc },
#else
		RecordBuilder() noexcept
#endif
		{
		}
		
#ifdef __cpp_lib_source_location
		RecordBuilder(std::string msg, const std::source_location& srcLoc = std::source_location::current()) :
			m_Record{ .sourceLocation = srcLoc },
#else
		RecordBuilder(std::string msg) :
#endif
			m_Stream{ std::move(msg) } // NOLINT(performance-move-const-arg)	new ostringstream ctor added in c++20
		{
		}

#ifdef __cpp_lib_source_location
		RecordBuilder(std::string_view msg, const std::source_location& srcLoc = std::source_location::current()) :
			m_Record{ .sourceLocation = srcLoc },
#else
		RecordBuilder(std::string_view msg)
#endif
		{
			m_Stream << msg;
		}

#ifdef __cpp_lib_source_location
		RecordBuilder(const char* msg, const std::source_location& srcLoc = std::source_location::current()) :
			m_Record{ .sourceLocation = srcLoc },
#else
		RecordBuilder(const char* msg)
#endif
		{
			m_Stream << msg;
		}

		~RecordBuilder() noexcept;

		RecordBuilder(RecordBuilder&& other) noexcept
		{
			*this = std::move(other);
		}

		RecordBuilder& operator =(RecordBuilder&& other) noexcept
		{
			using std::swap;
			swap(m_Record, other.m_Record);
			swap(m_Stream, other.m_Stream);
			m_Logger = std::exchange(other.m_Logger, nullptr);
			return *this;
		}

		RecordBuilder(const RecordBuilder&) = delete;
		RecordBuilder& operator =(const RecordBuilder&) = delete;

		template <class T>
		RecordBuilder& operator <<(T&& data)
		{
			m_Stream << std::forward<T>(data);
			return *this;
		}

		void setupTargetLogger(Logger& logger) noexcept
		{
			m_Logger = &logger;
		}

		[[nodiscard]] Record& record() noexcept
		{
			return m_Record;
		}

		[[nodiscard]] const Record& record() const noexcept
		{
			return m_Record;
		}

	private:
		Record m_Record;
		std::ostringstream m_Stream;
		Logger* m_Logger = nullptr;
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

		void log(Record rec)
		{
			assert(m_Core);
			m_Core->log(std::move(rec));
		}

		RecordBuilder makeRecordBuilder() noexcept
		{
			RecordBuilder builder;
			setupRecordBuilder(builder);
			return builder;
		}

		RecordBuilder operator <<(RecordBuilder builder) noexcept
		{
			setupRecordBuilder(builder);
			return builder;
		}

	private:
		Core* m_Core = nullptr;
		SeverityLevel m_DefaultSeverityLvl;
		std::any m_DefaultChannel;
		std::any m_DefaultUserData;

		void setupRecordBuilder(RecordBuilder& builder) noexcept
		{
			builder.record().severity = m_DefaultSeverityLvl;
			builder.record().channel = m_DefaultChannel;
			builder.record().userData = m_DefaultUserData;
			builder.setupTargetLogger(*this);
		}
	};

	inline RecordBuilder::~RecordBuilder() noexcept
	{
		if (m_Logger)
		{
			try
			{
				m_Record.message = std::move(m_Stream).str();
				m_Logger->log(std::move(m_Record));
			}
			catch (...)
			{
			}
		}
	}

	struct SetSeverity
	{
		std::any severity;
	};
	
	inline RecordBuilder& operator <<(RecordBuilder& recBuilder, SetSeverity setSev) noexcept
	{
		recBuilder.record().severity = std::move(setSev.severity);
		return recBuilder;
	}

	inline RecordBuilder operator <<(Logger& logger, SetSeverity setSev) noexcept
	{
		auto recBuilder = logger.makeRecordBuilder();
		recBuilder.record().severity = std::move(setSev.severity);
		return recBuilder;
	}

	struct SetChannel
	{
		std::any channel;
	};
	
	inline RecordBuilder& operator <<(RecordBuilder& recBuilder, SetChannel setChannel) noexcept
	{
		recBuilder.record().channel = std::move(setChannel.channel);
		return recBuilder;
	}

	inline RecordBuilder operator <<(Logger& logger, SetChannel setChannel) noexcept
	{
		auto recBuilder = logger.makeRecordBuilder();
		recBuilder.record().channel = std::move(setChannel.channel);
		return recBuilder;
	}

	struct SetUserData
	{
		std::any userData;
	};
	
	inline RecordBuilder& operator <<(RecordBuilder& recBuilder, SetUserData setUserData) noexcept
	{
		recBuilder.record().channel = std::move(setUserData.userData);
		return recBuilder;
	}

	inline RecordBuilder operator <<(Logger& logger, SetUserData setUserData) noexcept
	{
		auto recBuilder = logger.makeRecordBuilder();
		recBuilder.record().channel = std::move(setUserData.userData);
		return recBuilder;
	}
}

#endif
