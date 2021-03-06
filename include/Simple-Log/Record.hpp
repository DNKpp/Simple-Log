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
	/** \addtogroup Record
	 * @{
	 */

	/** \addtogroup RecordConcepts Record Concepts
	 * @{
	 * \details Users usually do not have to dig into the details of this part of the library, unless they try to inject an already existing but with incompatible interface Record type
	 * into the framework. Therefore the library provides several helper structs, which built a layer of abstraction onto the regular Record type interface, which users may specialize for their given types.
	 */

	/**
	 * \brief Provides a layer of abstraction to Record member types
	 * \tparam TRecord The used Record type
	 * \details There is no need for the users using this type of indirection. The library makes use of this abstraction, thus
	 * users may specialize it to use Record types, which does not fit in the original concept and trait syntax provided by this library.
	 * \version since alpha-0.7
	 */
	template <class TRecord>
	struct RecordTypedefs
	{
		using Message_t = typename TRecord::Message_t;
		using Severity_t = typename TRecord::SeverityLevel_t;
		using Channel_t = typename TRecord::Channel_t;
		using TimePoint_t = typename TRecord::TimePoint_t;
	};

	/**
	 * \typedef RecordMessage_t
	 * \brief Typedef for easier access to \ref Record "Record's" message type.
	 * \tparam TRecord The used Record type.
	 * \version since alpha-0.7
	 */
	template <class TRecord>
	/** \cond Requires */
	requires requires { typename RecordTypedefs<TRecord>::Message_t; }
	/** \endcond */
	using RecordMessage_t = typename RecordTypedefs<TRecord>::Message_t;

	/**
	 * \typedef RecordSeverity_t
	 * \brief Typedef for easier access to \ref Record "Record's" severity type.
	 * \tparam TRecord The used Record type.
	 * \version since alpha-0.7
	 */
	template <class TRecord>
	/** \cond Requires */
	requires requires { typename RecordTypedefs<TRecord>::Severity_t; }
	/** \endcond */
	using RecordSeverity_t = typename RecordTypedefs<TRecord>::Severity_t;

	/**
	 * \typedef RecordChannel_t
	 * \brief Typedef for easier access to \ref Record "Record's" channel type.
	 * \tparam TRecord The used Record type.
	 * \version since alpha-0.7
	 */
	template <class TRecord>
	/** \cond Requires */
	requires requires { typename RecordTypedefs<TRecord>::Channel_t; }
	/** \endcond */
	using RecordChannel_t = typename RecordTypedefs<TRecord>::Channel_t;

	/**
	 * \typedef RecordTimePoint_t
	 * \brief Typedef for easier access to \ref Record "Record's" time-point type.
	 * \tparam TRecord The used Record type.
	 * \version since alpha-0.7
	 */
	template <class TRecord>
	/** \cond Requires */
	requires requires { typename RecordTypedefs<TRecord>::TimePoint_t; }
	/** \endcond */
	using RecordTimePoint_t = typename RecordTypedefs<TRecord>::TimePoint_t;

	/**
	 * \brief Concept which checks for the necessary member typedefs of a Record type.
	 * \version since alpha-0.7
	 */
	template <class TRecord>
	concept RecordMemberTypedefs =
	requires
	{
		typename RecordMessage_t<TRecord>;
		typename RecordSeverity_t<TRecord>;
		typename RecordChannel_t<TRecord>;
		typename RecordTimePoint_t<TRecord>;
	};

	/**
	 * \brief Provides a layer of abstraction to Record member setter
	 * \tparam TRecord The used Record type
	 * \details There is no need for the users using this type of indirection. The library makes use of this abstraction, thus
	 * users may specialize it to use Record types, which does not fit in the original concept and trait syntax provided by this library.
	 * \version since alpha-0.7
	 */
	template <class TRecord>
	struct RecordGetters
	{
		constexpr static auto message{ [](auto& record) { return record.message(); } };
		constexpr static auto severity{ [](auto& record) { return record.severity(); } };
		constexpr static auto channel{ [](auto& record) { return record.channel(); } };
		constexpr static auto timePoint{ [](auto& record) { return record.timePoint(); } };
	};

	/**
	 * \brief Concept which checks for the necessary member projections of a Record type.
	 * \version since alpha-0.7
	 */
	template <class TRecord>
	concept RecordMemberGetters =
	RecordMemberTypedefs<TRecord> &&
	requires(const TRecord& rec)
	{
		{ RecordGetters<TRecord>::message(rec) } -> std::convertible_to<RecordMessage_t<TRecord>>;
		{ RecordGetters<TRecord>::severity(rec) } -> std::convertible_to<RecordSeverity_t<TRecord>>;
		{ RecordGetters<TRecord>::channel(rec) } -> std::convertible_to<RecordChannel_t<TRecord>>;
		{ RecordGetters<TRecord>::timePoint(rec) } -> std::convertible_to<RecordTimePoint_t<TRecord>>;
	};

	/**
	 * \brief Provides a layer of abstraction to Record member setter
	 * \tparam TRecord The used Record type
	 * \details There is no need for the users using this type of indirection. The library makes use of this abstraction, thus
	 * users may specialize it to use Record types, which does not fit in the original concept and trait syntax provided by this library.
	 * \version since alpha-0.7
	 */
	template <class TRecord>
	struct RecordSetters
	{
		using Record_t = std::remove_cvref_t<TRecord>;

		constexpr static auto setMessage
		{
			[]<class TMessage>(Record_t& record, TMessage&& msg)
			{
				return record.setMessage(std::forward<TMessage>(msg));
			}
		};

		constexpr static auto setSeverity
		{
			[]<class TSeverity>(Record_t& record, TSeverity&& sev)
			{
				return record.setSeverity(std::forward<TSeverity>(sev));
			}
		};

		constexpr static auto setChannel
		{
			[]<class TChannel>(Record_t& record, TChannel&& chan)
			{
				return record.setChannel(std::forward<TChannel>(chan));
			}
		};

		constexpr static auto setTimePoint
		{
			[]<class TTimePoint>(Record_t& record, TTimePoint&& timePoint)
			{
				return record.setTimePoint(std::forward<TTimePoint>(timePoint));
			}
		};
	};

	/**
	 * \brief Concept which checks for the necessary member setters of a Record type.
	 * \version since alpha-0.7
	 */
	template <class TRecord>
	concept RecordMemberSetters =
	RecordMemberTypedefs<TRecord> &&
	requires(TRecord& rec)
	{
		{ RecordSetters<TRecord>::setMessage(rec, std::declval<RecordMessage_t<TRecord>>()) };
		{ RecordSetters<TRecord>::setTimePoint(rec, std::declval<RecordTimePoint_t<TRecord>>()) };
		{ RecordSetters<TRecord>::setSeverity(rec, std::declval<RecordSeverity_t<TRecord>>()) };
		{ RecordSetters<TRecord>::setChannel(rec, std::declval<RecordChannel_t<TRecord>>()) };
	};

	/**
	 * \brief Concept which all the necessary concepts for Record types.
	 */
	template <class TRecord>
	concept Record =
	std::movable<TRecord> &&
	RecordMemberTypedefs<TRecord> &&
	RecordMemberGetters<TRecord> &&
	RecordMemberSetters<TRecord>;

	/** @}*/

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
		using Message_t = std::remove_cvref_t<TMessage>;
		using SeverityLevel_t = std::remove_cvref_t<TSeverityLevel>;
		using Channel_t = std::remove_cvref_t<TChannel>;
		using TimePoint_t = std::remove_cvref_t<TTimePoint>;
#ifdef __cpp_lib_source_location
		using SourceLocation_t = std::remove_cvref_t<std::source_location>;
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
