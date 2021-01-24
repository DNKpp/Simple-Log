//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_CORE_HPP
#define SL_LOG_CORE_HPP

#pragma once

#include <algorithm>
#include <execution>
#include <future>
#include <memory>
#include <mutex>
#include <vector>

#include "ISink.hpp"
#include "Record.hpp"
#include "RecordQueue.hpp"

namespace sl::log
{
	/**
	 * \brief This class is the central point of the whole library. It acts like a broker between the (multiple) Logger s on the frontend and the (multiple) Sink s on the backend. Due to this there must at least one
	 * instance of Core during the whole program runtime, but it isn't restricted to exist uniquely. It can be totally fine to create one global Core, which will be used for general logging purposes, and multiple others for a
	 * much lesser scope, like state logging on entity level.
	 *
	 * \details Each instance of Core consists of the following:
	 * \li one BlockingQueue in which all Records will get added
	 * \li multiple Sink objects
	 * \li one Worker thread, which will pull Records from that BlockingQueue and hand them over to the Sinks
	 *
	 * Due to this the Core is thread-safe by design.
	 *
	 * It is fine to add Sinks during later stages of your program. This hasn't necessarily to be done right after Core's creation. When Core goes out of scope, or is about to get destructed otherwise, it will block any new Records which could be
	 * pushed into but will also let the Worker finish its work. Therefor it will wait in the destructor, unless there is an exception throwing, in which case Core will force the Worker to quit its work instantly.
	 *
	 * Core instances are neither copy- nor movable.
	 */
	class Core
	{
	public:
		/**
		 * \brief Default Constructor
		 * \details The internal Worker thread will directly start running.
		 */
		Core() noexcept :
			m_Worker{ m_WorkerInstruction, m_Records, m_SinkMx, m_Sinks }
		{
			m_WorkerFuture = std::async(std::launch::async, m_Worker);
		}

		/**
		 * \brief Destructor
		 * \details Will block until the internal Record queue is empty or an exception rises, which will then force the Worker thread to quit.
		 */
		~Core() noexcept
		{
			try
			{
				m_WorkerInstruction = Instruction::quit;
				m_WorkerFuture.wait();
			}
			catch (...)
			{
				m_WorkerInstruction = Instruction::forceQuit;
			}
		}

		/**
		 * \brief Deleted copy constructor
		 */
		Core(const Core&) = delete;
		/**
		 * \brief Deleted copy assign operator
		 */
		Core& operator =(const Core&) = delete;

		/**
		 * \brief Deleted move constructor
		 */
		Core(Core&&) = delete;
		/**
		 * \brief Deleted move assign operator
		 */
		Core& operator =(Core&&) = delete;

		/**
		 * \brief Queues the Record internally
		 * \details This function should not be called directly on logging purposes. It serves as a simple interface for the corresponding Logger objects.
		 * \param record The record which will be queued
		 */
		void log(Record record)
		{
			// will reject newly generated records, after run has become false
			if (m_WorkerInstruction == Instruction::run)
			{
				m_Records.push(std::move(record));
			}
		}

		/**
		 * \brief Sink factory function
		 * \details This function creates a new Sink object and returns a reference to the caller. This Sink will be linked to and managed by the called Core instance.
		 * \tparam TSink Concrete Sink type
		 * \tparam TArgs Constructor argument types (will be deducted automatically)
		 * \param args The constructor arguments for the newly generated Sink object. Will be forwarded as is.
		 * \return reference to the managed Sink object
		 */
		template <class TSink, class... TArgs>
		TSink& makeSink(TArgs&&... args)
		{
			auto sink = std::make_unique<TSink>(std::forward<TArgs>(args)...);
			auto& ref = *sink;
			std::scoped_lock lock{ m_SinkMx };
			m_Sinks.emplace_back(std::move(sink));
			return ref;
		}

	private:
		using SinkContainer = std::vector<std::unique_ptr<ISink>>;

		enum class Instruction
		{
			run,
			quit,
			forceQuit
		};

		class Worker
		{
		public:
			Worker(
				const std::atomic<Instruction>& instruction,
				RecordQueue& records,
				std::mutex& sinkMx,
				const SinkContainer& sinks
			) :
				m_Instruction{ instruction },
				m_Records{ records },
				m_SinkMx{ sinkMx },
				m_Sinks{ sinks }
			{
			}

			void operator ()() const
			{
				for (auto instruction = m_Instruction.load();
					instruction != Instruction::forceQuit &&
					(instruction != Instruction::quit || !m_Records.isEmpty());
					instruction = m_Instruction)
				{
					if (auto optRecord = m_Records.take(std::chrono::milliseconds{ 200 }))
					{
						std::scoped_lock lock{ m_SinkMx };
						std::for_each(
									std::execution::par,
									std::begin(m_Sinks),
									std::end(m_Sinks),
									[&record = *optRecord](auto& sink)
									{
										sink->log(record);
									}
									);
					}
				}
			}

		private:
			const std::atomic<Instruction>& m_Instruction;

			RecordQueue& m_Records;

			std::mutex& m_SinkMx;
			const SinkContainer& m_Sinks;
		};

		RecordQueue m_Records;

		std::mutex m_SinkMx;
		SinkContainer m_Sinks;

		std::atomic<Instruction> m_WorkerInstruction{ Instruction::run };
		Worker m_Worker;
		std::future<void> m_WorkerFuture;
	};
}

#endif
