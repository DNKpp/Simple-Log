//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_RECORD_QUEUE_HPP
#define SL_LOG_RECORD_QUEUE_HPP

#pragma once

#include "Record.hpp"

#include <queue>
#include <mutex>
#include <optional>
#include <condition_variable>
#include <chrono>

namespace sl::log
{
	/** \addtogroup record
	 * @{
	 */
	
	class RecordQueue
	{
	public:
		void push(Record record)
		{
			{
				std::scoped_lock lock{ m_RecordMx };
				m_QueuedRecords.emplace(std::move(record));
			}
			m_PushVar.notify_one();
		}

		std::optional<Record> take(std::optional<std::chrono::milliseconds> waitingDuration = std::nullopt)
		{
			auto isQueueNotEmpty = [&records = m_QueuedRecords](){ return !std::empty(records); };
			
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

		bool isEmpty() const noexcept
		{
			std::scoped_lock lock{ m_RecordMx };
			return std::empty(m_QueuedRecords);
		}
	
	private:
		std::optional<Record> takeNextAsOpt()
		{
			auto record = std::move(m_QueuedRecords.front());
			m_QueuedRecords.pop();
			return std::optional<Record>{ std::in_place, std::move(record) };
		}
		
		mutable std::mutex m_RecordMx;
		std::queue<Record> m_QueuedRecords;
		std::condition_variable m_PushVar;
	};
	
	/** @}*/
}

#endif
