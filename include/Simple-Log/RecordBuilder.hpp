#//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_RECORD_BUILDER_HPP
#define SL_LOG_RECORD_BUILDER_HPP

#pragma once

#include <any>
#include <chrono>
#include <functional>
#include <sstream>

#include "Record.hpp"

namespace sl::log
{
	struct SetSeverity
	{
		template <class T>
		explicit SetSeverity(T&& data) :
			severity{ std::forward<T>(data) }
		{
		}

		std::any severity;
	};

	using SetSev = SetSeverity;

	struct SetChannel
	{
		template <class T>
		explicit SetChannel(T&& data) :
			channel{ std::forward<T>(data) }
		{
		}

		std::any channel;
	};

	using SetChan = SetChannel;

	struct SetUserData
	{
		template <class T>
		explicit SetUserData(T&& data) :
			userData{ std::forward<T>(data) }
		{
		}

		std::any userData;
	};

	using SetData = SetUserData;

	class RecordBuilder
	{
	public:
		using LogCallback_t = std::function<void(Record)>;

		explicit RecordBuilder(LogCallback_t cb) noexcept :
			m_Record{ .time = std::chrono::system_clock::now() },
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
			m_LogCallback = std::exchange(other.m_LogCallback, {});
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
		requires requires(T&& data)
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
		LogCallback_t m_LogCallback;
	};
}

#endif
