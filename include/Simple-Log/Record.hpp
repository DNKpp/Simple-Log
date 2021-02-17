//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_RECORD_HPP
#define SL_LOG_RECORD_HPP

#pragma once

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
	 * \brief A collection of logging related information
	 * \tparam TSeverityLevel Severity level type
	 * \tparam TChannel Channel descriptor type
	 * \tparam TMessage String type for storing messages
	 * \tparam TTimePoint Type for storing time related information
	 * 
	 * \details This class serves as a simple collection of gathered information during a logging action. It is a basic implementation of the Record concept
	 * and users are free to use this class as a starting point for creating expanded sub-classes or even create their own right from scratch. 
	 */
	template <std::semiregular TSeverityLevel,
		std::semiregular TChannel,
		std::semiregular TMessage = std::string,
		std::semiregular TTimePoint = std::chrono::system_clock::time_point>
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

		/**
		 * \brief Const access to the message
		 * \return Returns the stored message
		 */
		[[nodiscard]]
		const Message_t& message() const noexcept
		{
			return m_Message;
		}

		/**
		 * \brief Sets the message string
		 * \tparam UMessage Type which must be convertible to Message_t
		 * \param msg Message content
		 */
		template <std::convertible_to<Message_t> UMessage>
		void setMessage(UMessage&& msg)
		{
			m_Message = std::forward<UMessage>(msg);
		}

		/**
		 * \brief Const access to the time stamp
		 * \return Returns a const ref
		 */
		[[nodiscard]]
		const TimePoint_t& timePoint() const noexcept
		{
			return m_TimePoint;
		}

		/**
		 * \brief Sets the Record s time stamp
		 * \tparam UTimePoint Type which must be convertible to TimePoint_t
		 * \param timePoint A time point
		 */
		template <std::convertible_to<TimePoint_t> UTimePoint>
		void setTimePoint(UTimePoint&& timePoint)
		{
			m_TimePoint = std::forward<UTimePoint>(timePoint);
		}

		/**
		 * \brief Const access to the severity level
		 * \return Returns a const ref
		 */
		[[nodiscard]]
		const SeverityLevel_t& severity() const noexcept
		{
			return m_Severity;
		}

		/**
		 * \brief Sets the Record s severity level
		 * \tparam USeverityLevel Type which must be convertible to SeverityLevel_t
		 * \param severity A time point
		 */
		template <std::convertible_to<SeverityLevel_t> USeverityLevel>
		void setSeverity(USeverityLevel&& severity)
		{
			m_Severity = std::forward<USeverityLevel>(severity);
		}

		/**
		 * \brief Const access to the channel
		 * \return Returns a const ref
		 */
		[[nodiscard]]
		const Channel_t& channel() const noexcept
		{
			return m_Channel;
		}

		/**
		 * \brief Sets the Record s severity level
		 * \tparam UChannel Type which must be convertible to Channel_t
		 * \param channel channel descriptor
		 */
		template <std::convertible_to<Channel_t> UChannel>
		void setChannel(UChannel&& channel)
		{
			m_Channel = std::forward<UChannel>(channel);
		}

#ifdef __cpp_lib_source_location
		/**
		 * \brief Const access to the source location
		 * \return Returns a const ref
		 */
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
		SourceLocation_t m_SourceLocation{};
#endif
	};

	/** @}*/
}

#endif
