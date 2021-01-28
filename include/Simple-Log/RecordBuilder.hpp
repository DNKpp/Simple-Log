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
	/** \addtogroup record
	 * @{
	 */

	/** \addtogroup RecordBuilder
	 * @{
	 */

	/**
	 * \brief Manipulates the severity level of the current RecordBuilder object
	 * \details This type is generally designed to be directly used in logging expressions and just stores data which will be hand-over to a RecordBuilder instance.
	 * In fact this is just a helper struct for which RecordBuilder provides an overload of operator <<, which will then modify the severity level of the current RecordBuilder object.
	 * \ingroup Test
	 */
	struct SetSeverity
	{
		/**
		 * \brief Constructor accepting severity data
		 * \tparam T T must be implicit convertible to std::any.
		 * \param data Severity data.
		 */
		template <class T>
		explicit SetSeverity(T&& data) :
			severity{ std::forward<T>(data) }
		{
		}

		std::any severity;
	};

	/**
	 * \brief Manipulates the severity level of the current RecordBuilder object
	 * \details This is a typedef for SetSeverity
	 */
	using SetSev = SetSeverity;

	/**
	 * \brief Manipulates the channel of the current RecordBuilder object
	 * \details This type is generally designed to be directly used in logging expressions and just stores data which will be hand-over to a RecordBuilder instance.
	 * In fact this is just a helper struct for which RecordBuilder provides an overload of operator <<, which will then modify the channel of the current RecordBuilder object.
	 */
	struct SetChannel
	{
		/**
		 * \brief Constructor accepting channel data
		 * \tparam T T must be implicit convertible to std::any.
		 * \param data Channel data.
		 */
		template <class T>
		explicit SetChannel(T&& data) :
			channel{ std::forward<T>(data) }
		{
		}

		std::any channel;
	};

	/**
	 * \brief Manipulates the channel of the current RecordBuilder object
	 * \details This is a typedef for SetChannel
	 */
	using SetChan = SetChannel;

	/**
	 * \brief Manipulates the user data of the current RecordBuilder object
	 * \details This type is generally designed to be directly used in logging expressions and just stores data which will be hand-over to a RecordBuilder instance.
	 * In fact this is just a helper struct for which RecordBuilder provides an overload of operator <<, which will then modify the user data of the current RecordBuilder object.
	 */
	struct SetUserData
	{
		/**
		 * \brief Constructor accepting user data
		 * \tparam T T must be implicit convertible to std::any.
		 * \param data User data.
		 */
		template <class T>
		explicit SetUserData(T&& data) :
			userData{ std::forward<T>(data) }
		{
		}

		std::any userData;
	};

	/**
	 * \brief Manipulates the user data of the current RecordBuilder object
	 * \details This is a typedef for SetUserData
	 */
	using SetData = SetUserData;

	/**
	 * \brief Helper class for building new Records
	 * \details This is class provides the simple and elegant interface for making logging expressions. Its objects are non-copyable but movable.
	 * When a RecordBuilder object gets destroyed (mainly because going out of scope) it will automatically send its created Record to the designated Logger object. Users should not instantiate
	 * objects themselves, but should instead use the Logger objects.
	 */
	class RecordBuilder
	{
	private:
		using LogCallback_t = std::function<void(Record)>;

	public:
		/**
		 * \brief Constructor
		 * \details This constructor requires a callback to the associated Logger objects log function. The callback should have the following signature:
		 * \code
		 * void(Record)
		 * \endcode
		 * \param cb Callback to the associated Logger object
		 */
		explicit RecordBuilder(LogCallback_t cb) noexcept :
			m_Record{ .time = std::chrono::system_clock::now() },
			m_LogCallback{ std::move(cb) }
		{
		}

		/**
		 * \brief Destructor
		 * \details If RecordBuilder is valid (in fact, if a callback is installed) it moves its created Record object to the Logger object.
		 */
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

		/**
		 * \brief Move constructor
		 * \details Transfers ownership of other's data to this object. If other has an installed callback, it will be cleared.
		 * \param other Another RecordBuilder object
		 */
		RecordBuilder(RecordBuilder&& other) noexcept
		{
			*this = std::move(other);
		}

		/**
		 * \brief Move assignment operator
		 * \details Transfers ownership of other's data to this object. If other has an installed callback, it will be cleared.
		 * \param other Another RecordBuilder object
		 * \return Returns a reference to this.
		 */
		RecordBuilder& operator =(RecordBuilder&& other) noexcept
		{
			using std::swap;
			swap(m_Record, other.m_Record);
			swap(m_Stream, other.m_Stream);
			m_LogCallback = std::exchange(other.m_LogCallback, {});
			return *this;
		}

		/**
		 * \brief Deleted copy constructor
		 */
		RecordBuilder(const RecordBuilder&) = delete;
		/**
		 * \brief Deleted copy assign operator
		 */
		RecordBuilder& operator =(const RecordBuilder&) = delete;

		/**
		 * \brief Accessor to the internal record object
		 */
		[[nodiscard]] Record& record() noexcept
		{
			return m_Record;
		}

		/**
		 * \brief Const accessor to the internal record object
		 */
		[[nodiscard]] const Record& record() const noexcept
		{
			return m_Record;
		}

		/**
		 * \brief Output operator
		 * \details Attaches data to the underlying ostream object.
		 * \tparam T Requires that std::ostringstream has an overload of operator << for T.
		 * \param data Data about to be attached.
		 * \return Returns a reference to this.
		 */
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

		/**
		 * \brief Special output operator overload for SetSeverity
		 * \details Manipulates the severity level of the Record created by this RecordBuilder instance.
		 * \param setSev Holds the concrete severity level.
		 * \return Returns a reference to this.
		 */
		RecordBuilder& operator <<(SetSeverity setSev) noexcept
		{
			m_Record.severity = std::move(setSev.severity);
			return *this;
		}

		/**
		 * \brief Special output operator overload for SetChannel
		 * \details Manipulates the channel of the Record created by this RecordBuilder instance.
		 * \param setChannel Holds the concrete channel.
		 * \return Returns a reference to this.
		 */
		RecordBuilder& operator <<(SetChannel setChannel) noexcept
		{
			m_Record.channel = std::move(setChannel.channel);
			return *this;
		}

		/**
		 * \brief Special output operator overload for SetUserData
		 * \details Manipulates the user data of the Record created by this RecordBuilder instance.
		 * \param setUserData Holds the concrete user data.
		 * \return Returns a reference to this.
		 */
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

	/** @}*/
	/** @}*/
}

#endif
