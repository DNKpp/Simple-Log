#//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_RECORD_BUILDER_HPP
#define SL_LOG_RECORD_BUILDER_HPP

#pragma once

#include <functional>
#include <sstream>

#include "Concepts.hpp"

namespace sl::log
{
	/** \addtogroup Record
	 * @{
	 */

	/**
	 * \brief Manipulates the channel of the current RecordBuilder object
	 * \tparam TSeverityLevel Severity level type which must later on be convertible to Record's severity level type.
	 * \details This type is generally designed to be directly used in logging expressions. When handed-over to a RecordBuilder instance by operator <<,
	 * it will then manipulate the severity level data of its constructing Record object.
	 */
	template <class TSeverityLevel>
	class SetSev
	{
	public:
		/**
		 * \brief Constructor accepting severity level data
		 * \param data Severity level data.
		 */
		explicit constexpr SetSev(TSeverityLevel data) noexcept(std::is_nothrow_move_constructible_v<TSeverityLevel>) :
			m_Data{ std::move(data) }
		{
		}

		/**
		 * \brief Changes severity level of the passed Record object
		 * \tparam TRecord Used Record type.
		 * \param rec The Record which is about to change
		 */
		template <Record TRecord>
		void operator ()(TRecord& rec) const
		{
			rec.setSeverity(m_Data);
		}

	private:
		TSeverityLevel m_Data;
	};

	/**
	 * \brief Manipulates the channel of the current RecordBuilder object
	 * \tparam TChannel Channel type which must later on be convertible to Record's channel type.
	 * \details This type is generally designed to be directly used in logging expressions. When handed-over to a RecordBuilder instance by operator <<,
	 * it will then manipulate the Channel data of its constructing Record object.
	 */
	template <class TChannel>
	class SetChan
	{
	public:
		/**
		 * \brief Constructor accepting channel data
		 * \param data Channel data.
		 */
		explicit constexpr SetChan(TChannel data) noexcept(std::is_nothrow_move_constructible_v<TChannel>) :
			m_Data{ std::move(data) }
		{
		}

		/**
		 * \brief Changes channel of the passed Record object
		 * \tparam TRecord Used Record type.
		 * \param rec The Record which is about to change
		 */
		template <Record TRecord>
		void operator ()(TRecord& rec) const
		{
			rec.setChannel(m_Data);
		}

	private:
		TChannel m_Data;
	};

	/**
	 * \brief Helper class for building new Records
	 * \tparam TRecord Used Record type.
	 * \details This is class provides the simple and elegant interface for making logging expressions. Its objects are non-copyable but movable.
	 * When a RecordBuilder object gets destroyed (mainly because going out of scope) it will automatically send its created Record to the designated Logger object. Users should not instantiate
	 * objects themselves, but should instead use the Logger objects.
	 */
	template <Record TRecord>
	class RecordBuilder
	{
	public:
		using Record_t = TRecord;
		using SeverityLevel_t = typename Record_t::SeverityLevel_t;
		using Channel_t = typename Record_t::Channel_t;

	private:
		using LogCallback_t = std::function<void(Record_t)>;

	public:
		/**
		 * \brief Constructor
		 * \param prefabRec Prefabricated Record
		 * \param cb Callback to the associated Logger object
		 * \details This constructor requires a callback to the associated Logger objects log function. The callback should have the following signature:
		 * \code
		 * void(Record)
		 * \endcode
		 */
		explicit RecordBuilder(Record_t prefabRec, LogCallback_t cb) noexcept :
			m_Record{ std::move(prefabRec) },
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
					m_Record.setMessage(std::move(m_Stream).str());
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
		 * \brief Move-assign operator
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
		 * \brief Deleted copy-assign operator
		 */
		RecordBuilder& operator =(const RecordBuilder&) = delete;

		/**
		 * \brief Accessor to the internal record object
		 */
		[[nodiscard]]
		Record_t& record() noexcept
		{
			return m_Record;
		}

		/**
		 * \brief Const accessor to the internal record object
		 */
		[[nodiscard]]
		const Record_t& record() const noexcept
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
		 * \brief Special output operator overload for actions on Record
		 * \param action Action which will be applied to the internal Record object.
		 * \return Returns a reference to this.
		 */
		template <std::invocable<Record_t&> TAction>
		RecordBuilder& operator <<(TAction action)
		{
			std::invoke(action, m_Record);
			return *this;
		}

	private:
		Record_t m_Record;
		std::ostringstream m_Stream;
		LogCallback_t m_LogCallback;
	};

	/** @}*/
}

#endif
