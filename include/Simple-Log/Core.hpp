//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_CORE_HPP
#define SL_LOG_CORE_HPP

#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <algorithm>
#include <execution>
#include <future>

#include "ISink.hpp"
#include "Record.hpp"
#include "RecordQueue.hpp"

namespace sl::log
{	
	class Core
	{
	public:
		Core() noexcept :
			m_Worker{ m_WorkerRun, m_Records, m_SinkMx, m_Sinks }
		{
			m_WorkerFuture = std::async(std::launch::async, m_Worker);
		}

		~Core()
		{
			m_WorkerRun = false;
			m_WorkerFuture.wait();
		}
		
		void log(Record record)
		{
			// we will reject newly generated records, after run has become false
			if (m_WorkerRun)
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
		
		class Worker
		{
		public:
			Worker(const std::atomic_bool& run, RecordQueue& records, std::mutex& sinkMx, const SinkContainer& sinks) :
				m_Run{ run },
				m_Records{ records },
				m_SinkMx{ sinkMx },
				m_Sinks{ sinks }
			{
			}

			void operator ()() const
			{
				while (m_Run || !m_Records.isEmpty())
				{
					if (auto optRecord = m_Records.take(std::chrono::milliseconds{ 200 }))
					{
						std::scoped_lock lock{ m_SinkMx };
						std::for_each(std::execution::par, std::begin(m_Sinks), std::end(m_Sinks),
							[&record = *optRecord](auto& sink)
							{
								sink->log(record);
							}
						);
					}
				}
			}
		
		private:
			const std::atomic_bool& m_Run;

			RecordQueue& m_Records;

			std::mutex& m_SinkMx;
			const SinkContainer& m_Sinks;
		};

		RecordQueue m_Records;

		std::mutex m_SinkMx;
		SinkContainer m_Sinks;

		std::atomic_bool m_WorkerRun{ true };
		Worker m_Worker;
		std::future<void> m_WorkerFuture;
	};
}

#endif
