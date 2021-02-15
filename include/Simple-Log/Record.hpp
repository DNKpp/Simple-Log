//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_RECORD_HPP
#define SL_LOG_RECORD_HPP

#pragma once

#include <chrono>
#include <ostream>
#include <string>
#include <tuple>
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
	 * \brief A collection of logging related information
	 * \details This class serves as a simple collection of all the gathered information during a logging action.
	 */
	template <std::semiregular TSeverityLevel,
			std::semiregular TChannel,
			std::semiregular TMessage = std::string,
			std::semiregular TTimePoint = std::chrono::system_clock::time_point
	>
	class BaseRecord
	{
	public:
		using Message_t = TMessage;
		using SeverityLevel_t = TSeverityLevel;
		using Channel_t = TChannel;
		using TimePoint_t = TTimePoint;
#ifdef __cpp_lib_source_location
		using SourceLocation_t = std::source_location;
#endif

		[[nodiscard]]
		std::string_view message() const noexcept
		{
			return m_Message;
		}

		template <std::convertible_to<Message_t> UMessage>
		void setMessage(UMessage&& msg)
		{
			m_Message = std::forward<UMessage>(msg);
		}

		[[nodiscard]]
		const TimePoint_t& timePoint() const noexcept
		{
			return m_TimePoint;
		}

		template <std::convertible_to<TimePoint_t> UTimePoint>
		void setTimePoint(UTimePoint&& timePoint)
		{
			m_TimePoint = std::forward<UTimePoint>(timePoint);
		}

		[[nodiscard]]
		const SeverityLevel_t& severity() const noexcept
		{
			return m_Severity;
		}

		template <std::convertible_to<SeverityLevel_t> USeverityLevel>
		void setSeverity(USeverityLevel&& severity)
		{
			m_Severity = std::forward<USeverityLevel>(severity);
		}

		[[nodiscard]]
		const Channel_t& channel() const noexcept
		{
			return m_Channel;
		}

		template <std::convertible_to<Channel_t> UChannel>
		void setChannel(UChannel&& channel)
		{
			m_Channel = std::forward<UChannel>(channel);
		}

#ifdef __cpp_lib_source_location
		[[nodiscard]]
		const SourceLocation_t& sourceLocation() const noexcept
		{
			return m_SourceLocation;
		}

		template <std::convertible_to<SourceLocation_t> USourceLocation>
		void setChannel(USourceLocation&& sourceLocation)
		{
			m_SourceLocation = std::forward<USourceLocation>(sourceLocation);
		}
#endif

	private:
		Message_t m_Message{};
		TimePoint_t m_TimePoint{};
		SeverityLevel_t m_Severity{};
		Channel_t m_Channel{};
#ifdef __cpp_lib_source_location
		SourceLocation_t m_SourceLocation;
#endif
	};

	/** @}*/
}

#endif
