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

#include "Concepts.hpp"
#include "ISink.hpp"
#include "RecordQueue.hpp"

namespace sl::log
{
	/**
	 * \brief The central point of the whole library. Needs to be instantiated at least once.
	 * \tparam TRecord Used Record type.
	 * 
	 * \details Objects of this class act like a broker between the (multiple) Logger s on the frontend and the (multiple) Sink s on the backend. Due to this there must at least one living
	 * instance of Core during the whole program runtime, but it isn't restricted to exist uniquely. It can be totally fine to create one global Core, which will be used for general logging purposes, and multiple others for a
	 * much lesser scope, like state logging on entity level. Logger will be permanently linked to one specific Core instance, thus Core instances must outlive their corresponding Logger s.
	 *
	 * Each instance of Core consists of the following:
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

	template <Record TRecord>
	class Core
	{
	public:
		using Record_t = TRecord;

	private:
		using ISink_t = ISink<Record_t>;
		using RecordQueue_t = RecordQueue<Record_t>;
		using SinkContainer_t = std::vector<std::unique_ptr<ISink_t>>;

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
		 * \param record The record which will be queued
		 * \details This function should not be called directly on logging purposes. It serves as a simple interface for the corresponding Logger objects.
		 */
		void log(Record_t record)
		{
			// will reject newly generated records, after run has become false
			if (m_WorkerInstruction == Instruction::run)
			{
				m_Records.push(std::move(record));
			}
		}

		/**
		 * \brief Creates Sink and registers it at this Core instance
		 * \tparam TSink Concrete Sink type
		 * \tparam TArgs Constructor argument types (will be deducted automatically)
		 * \param args The constructor arguments for the newly generated Sink object. Will be forwarded as is.
		 * \return reference to the managed Sink object
		 * \details This function creates a new Sink object and returns a reference to the caller. This Sink will be linked to and managed by the called Core instance.
		 */
		template <std::derived_from<ISink_t> TSink, class... TArgs>
		requires std::constructible_from<TSink, TArgs...>
		TSink& makeSink(TArgs&&... args)
		{
			auto& ref = makeSinkImpl<TSink>(std::forward<TArgs>(args)...);
			ref.enable();
			return ref;
		}

		/**
		 * \brief Creates Sink disabled and registers it at this Core instance
		 * \tparam TSink Concrete Sink type
		 * \tparam TArgs Constructor argument types (will be deducted automatically)
		 * \param args The constructor arguments for the newly generated Sink object. Will be forwarded as is.
		 * \return Wrapped reference to the managed Sink object.
		 * \details This function creates a new disabled Sink object and returns a wrapped reference to the caller. When this wrapper goes out of scope or gets destructed otherwise,
		 * the attached Sink will become enabled. Use this if you want to make sure, that no messages will be handled before your Sink is finally setup.
		 * This Sink will be linked to and managed by the called Core instance.
		 */
		template <std::derived_from<ISink_t> TSink, class... TArgs>
		requires std::constructible_from<TSink, TArgs...>
		ScopedSinkDisabling<Record_t, TSink> makeDisabledSink(TArgs&&... args)
		{
			auto& ref = makeSinkImpl<TSink>(std::forward<TArgs>(args)...);
			return ref;
		}

	private:
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
				RecordQueue_t& records,
				std::mutex& sinkMx,
				const SinkContainer_t& sinks
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

			RecordQueue_t& m_Records;

			std::mutex& m_SinkMx;
			const SinkContainer_t& m_Sinks;
		};

		RecordQueue_t m_Records;

		std::mutex m_SinkMx;
		SinkContainer_t m_Sinks;

		std::atomic<Instruction> m_WorkerInstruction{ Instruction::run };
		Worker m_Worker;
		std::future<void> m_WorkerFuture;

		template <std::derived_from<ISink_t> TSink, class... TArgs>
		requires std::constructible_from<TSink, TArgs...>
		TSink& makeSinkImpl(TArgs&&... args)
		{
			auto sink = std::make_unique<TSink>(std::forward<TArgs>(args)...);
			auto& ref = *sink;
			std::scoped_lock lock{ m_SinkMx };
			m_Sinks.emplace_back(std::move(sink));
			return ref;
		}
	};
}

#endif
