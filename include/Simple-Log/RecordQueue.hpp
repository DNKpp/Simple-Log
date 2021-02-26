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

namespace sl::log::detail
{
	template <Record TRecord>
	[[nodiscard]]
	auto takeNextAsOpt(std::queue<TRecord>& queue)
	{
		auto record = std::move(queue.front());
		queue.pop();
		return std::optional<TRecord>{ std::in_place, std::move(record) };
	}

	template <Record TRecord>
	[[nodiscard]]
	std::optional<TRecord> blockingTake(
		std::queue<TRecord>& queue,
		std::mutex& queueMx,
		std::condition_variable& condVar,
		const std::optional<std::chrono::milliseconds>& waitDur
	)
	{
		auto isQueueNotEmpty = [&records = queue]() { return !std::empty(records); };

		std::unique_lock lock{ queueMx };
		if (waitDur)
		{
			if (condVar.wait_for(lock, *waitDur, isQueueNotEmpty))
			{
				return detail::takeNextAsOpt(queue);
			}
			return std::nullopt;
		}

		condVar.wait(lock, isQueueNotEmpty);
		return detail::takeNextAsOpt(queue);
	}

	template <Record TRecord>
	class BlockingTake
	{
	public:
		using Record_t = std::remove_cvref_t<TRecord>;
		using Queue_t = std::queue<Record_t>;

		BlockingTake(std::mutex& queueMx, Queue_t& queue) :
			m_QueueMx{ queueMx },
			m_Queue{ queue }
		{
		}

		/**
		 * \brief Pushes \ref Record Records to the internal queue
		 * \tparam URecord Must be implicitly convertible to Record_t
		 * \param record The queued Record object.
		 */
		template <class URecord = Record_t&&>
		void push(URecord&& record)
		{
			{
				std::scoped_lock lock{ m_QueueMx };
				m_Queue.emplace(std::forward<URecord>(record));
			}
			m_PushVar.notify_one();
		}

		/**
		 * \brief Takes the first Record from the queue
		 * \param waitDur The max waiting duration for an element.
		 * \return Returns an element as optional. Might be std::nullopt.
		 * \details If the internal queue is not empty, this functions takes the first element and returns it. If the queue is empty, this function will block until a new Record
		 * gets pushed into the queue or the duration exceeds.
		 */
		[[nodiscard]]
		std::optional<Record_t> take(std::optional<std::chrono::milliseconds> waitDur = std::nullopt)
		{
			return detail::blockingTake(m_Queue, m_QueueMx, m_PushVar, waitDur);
		}

	private:
		std::mutex& m_QueueMx;
		Queue_t& m_Queue;
		std::condition_variable m_PushVar;
	};

	template <Record TRecord, std::size_t VMaxQueueSize>
	requires (0 < VMaxQueueSize)
	class BlockingPushTake
	{
	public:
		using Record_t = std::remove_cvref_t<TRecord>;
		using Queue_t = std::queue<Record_t>;

		BlockingPushTake(std::mutex& queueMx, Queue_t& queue) :
			m_QueueMx{ queueMx },
			m_Queue{ queue }
		{
		}

		/**
		 * \brief Inserts the given record into the queue or discards it if queue is full
		 * \tparam URecord Must be implicitly convertible to Record_t
		 * \param record The Record which is about to be inserted
		 */
		template <class URecord = Record_t&&>
		void push(URecord&& record)
		{
			auto isQueueNotFull = [&queue = m_Queue]()
			{
				return std::size(queue) < VMaxQueueSize;
			};

			std::unique_lock lock{ m_QueueMx };
			m_TakeVar.wait(lock, isQueueNotFull);
			m_Queue.emplace(std::forward<URecord>(record));
			m_PushVar.notify_one();
		}

		/**
		 * \brief Takes the first Record from the queue
		 * \param waitDur The max waiting duration for an element.
		 * \return Returns an element as optional. Might be std::nullopt.
		 * \details If the internal queue is not empty, this functions takes the first element and returns it. If the queue is empty, this function will block until a new Record
		 * gets pushed into the queue or the duration exceeds.
		 */
		[[nodiscard]]
		std::optional<Record_t> take(std::optional<std::chrono::milliseconds> waitDur = std::nullopt)
		{
			auto result = detail::blockingTake(m_Queue, m_QueueMx, m_PushVar, waitDur);
			if (result)
			{
				m_TakeVar.notify_one();
			}
			return result;
		}

	private:
		std::mutex& m_QueueMx;
		Queue_t& m_Queue;
		std::condition_variable m_PushVar;
		std::condition_variable m_TakeVar;
	};

	template <Record TRecord, std::size_t VMaxQueueSize>
	requires (0 < VMaxQueueSize)
	class DiscardedPushBlockingTake
	{
	public:
		using Record_t = std::remove_cvref_t<TRecord>;
		using Queue_t = std::queue<Record_t>;

		DiscardedPushBlockingTake(std::mutex& queueMx, Queue_t& queue) :
			m_QueueMx{ queueMx },
			m_Queue{ queue }
		{
		}

		/**
		 * \brief Inserts the given record into the queue or waits if queue is full
		 * \tparam URecord Must be implicitly convertible to Record_t
		 * \param record The Record which is about to be inserted
		 */
		template <class URecord = Record_t&&>
		void push(URecord record)
		{
			if (std::scoped_lock lock{ m_QueueMx }; std::size(m_Queue) < VMaxQueueSize)
			{
				m_Queue.emplace(std::forward<URecord>(record));
			}
			else
				return;

			m_PushVar.notify_one();
		}

		/**
		 * \brief Takes the first Record from the queue
		 * \param waitDur The max waiting duration for an element.
		 * \return Returns an element as optional. Might be std::nullopt.
		 * \details If the internal queue is not empty, this functions takes the first element and returns it. If the queue is empty, this function will block until a new Record
		 * gets pushed into the queue or the duration exceeds.
		 */
		[[nodiscard]]
		std::optional<Record_t> take(std::optional<std::chrono::milliseconds> waitDur = std::nullopt)
		{
			return detail::blockingTake(m_Queue, m_QueueMx, m_PushVar, waitDur);
		}

	private:
		std::mutex& m_QueueMx;
		Queue_t& m_Queue;
		std::condition_variable m_PushVar;
	};
}

namespace sl::log
{
	/** \addtogroup Core
	 * @{
	 */

	/**
	 * \brief Storage for \ref Record Records
	 * \tparam TRecord Used Record type.
	 * \details This class is a simple representation of a blocking queue. Its take() function blocks until an element is present in the internal queue or
	 * the duration exceeded. Each function is thread-safe by design.
	 */
	template <Record TRecord, QueueStrategyFor<TRecord> TQueueStrategy = detail::BlockingTake<TRecord>>
	class RecordQueue :
		public TQueueStrategy
	{
		using Super = TQueueStrategy;

	public:
		using Record_t = std::remove_cvref_t<TRecord>;

		RecordQueue() :
			Super{ m_RecordMx, m_QueuedRecords }
		{
		}

		/**
		 * \brief Checks if the internal queue is empty
		 * \details Thread-safe
		 * \return Returns true if is empty.
		 */
		[[nodiscard]]
		bool empty() const noexcept
		{
			std::scoped_lock lock{ m_RecordMx };
			return std::empty(m_QueuedRecords);
		}

		/**
		 * \brief Checks size of the internal queue
		 * \details Thread-safe
		 * \return Returns size
		 */
		[[nodiscard]]
		std::size_t size() const noexcept
		{
			std::scoped_lock lock{ m_RecordMx };
			return std::size(m_QueuedRecords);
		}

	private:
		mutable std::mutex m_RecordMx;
		std::queue<Record_t> m_QueuedRecords;
	};

	struct BlockingTakeQueueStrategy
	{
	};

	template <std::size_t VMaxQueueSize>
	struct BlockingPushTakeQueueStrategy
	{
	};

	template <std::size_t VMaxQueueSize>
	struct DiscardingPushBlockingTakeQueueStrategy
	{
	};

	/** @}*/
}

namespace sl::log::detail
{
	template <Record TRecord, class TUnknownStrategy>
	struct MakeRecordQueueHelper
	{
	};

	template <Record TRecord>
	struct MakeRecordQueueHelper<TRecord, BlockingTakeQueueStrategy>
	{
		using RecordQueue_t = RecordQueue<TRecord, BlockingTake<TRecord>>;
	};

	template <Record TRecord, std::size_t VMaxSize>
	struct MakeRecordQueueHelper<TRecord, DiscardingPushBlockingTakeQueueStrategy<VMaxSize>>
	{
		using RecordQueue_t = RecordQueue<TRecord, DiscardedPushBlockingTake<TRecord, VMaxSize>>;
	};

	template <Record TRecord, std::size_t VMaxSize>
	struct MakeRecordQueueHelper<TRecord, BlockingPushTakeQueueStrategy<VMaxSize>>
	{
		using RecordQueue_t = RecordQueue<TRecord, BlockingPushTake<TRecord, VMaxSize>>;
	};
}

#endif
