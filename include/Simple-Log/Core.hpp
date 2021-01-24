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
	class Core
	{
	public:
		Core() noexcept :
			m_Worker{ m_WorkerInstruction, m_Records, m_SinkMx, m_Sinks }
		{
			m_WorkerFuture = std::async(std::launch::async, m_Worker);
		}

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

		Core(const Core&) = delete;
		Core& operator =(const Core&) = delete;

		Core(Core&&) = delete;
		Core& operator =(Core&&) = delete;

		void log(Record record)
		{
			// will reject newly generated records, after run has become false
			if (m_WorkerInstruction == Instruction::run)
			{
				m_Records.push(std::move(record));
			}
		}

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
