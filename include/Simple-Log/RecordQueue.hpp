//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_RECORD_QUEUE_HPP
#define SL_LOG_RECORD_QUEUE_HPP

#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

#include "Concepts.hpp"

namespace sl::log
{
	/** \addtogroup record
	 * @{
	 */

	/**
	 * \brief Storage for Record s
	 * \details This class is a simple representation of a blocking queue. Its take() function blocks until an element is present in the internal queue or
	 * the duration exceeded. Each function is thread-safe by design.
	 */
	template <Record TRecord>
	class RecordQueue
	{
	public:
		using Record_t = TRecord;

		/**
		 * \brief Pushes Record s to the internal queue
		 * \details Thread-safe
		 * \param record The queued Record object.
		 */
		void push(Record_t record)
		{
			{
				std::scoped_lock lock{ m_RecordMx };
				m_QueuedRecords.emplace(std::move(record));
			}
			m_PushVar.notify_one();
		}

		/**
		 * \brief Takes the first Record from the queue
		 * \details Thread-safe
		 * If the internal queue is not empty, this functions takes the first element and returns it. If the queue is empty, this function will block until a new Record gets pushed into the queue
		 * or the duration exceeds.
		 * \param waitingDuration The max waiting duration for an element.
		 * \return Returns an element as optional. Might be nullopt.
		 */
		std::optional<Record_t> take(std::optional<std::chrono::milliseconds> waitingDuration = std::nullopt)
		{
			auto isQueueNotEmpty = [&records = m_QueuedRecords]() { return !std::empty(records); };

			std::unique_lock lock{ m_RecordMx };
			if (waitingDuration)
			{
				if (m_PushVar.wait_for(lock, *waitingDuration, isQueueNotEmpty))
				{
					return takeNextAsOpt();
				}
				return std::nullopt;
			}

			m_PushVar.wait(lock, isQueueNotEmpty);
			return takeNextAsOpt();
		}

		/**
		 * \brief Checks if the internal queue is empty
		 * \details Thread-safe
		 * \return Returns true if is empty.
		 */
		bool isEmpty() const noexcept
		{
			std::scoped_lock lock{ m_RecordMx };
			return std::empty(m_QueuedRecords);
		}

	private:
		std::optional<Record_t> takeNextAsOpt()
		{
			auto record = std::move(m_QueuedRecords.front());
			m_QueuedRecords.pop();
			return std::optional<Record_t>{ std::in_place, std::move(record) };
		}

		mutable std::mutex m_RecordMx;
		std::queue<Record_t> m_QueuedRecords;
		std::condition_variable m_PushVar;
	};

	/** @}*/
}

#endif
