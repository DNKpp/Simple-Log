//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_BASIC_SINK_HPP
#define SL_LOG_BASIC_SINK_HPP

#pragma once

#include <functional>
#include <mutex>
#include <ostream>
#include <string>

#include "Concepts.hpp"
#include "ISink.hpp"

namespace sl::log
{
	using Formatter_t = std::function<void(std::ostream&, const Record&)>;
	using Filter_t = std::function<bool(const Record&)>;

	class BasicSink :
		public ISink
	{
	protected:
		static auto defaultFormatter() noexcept
		{
			return [](std::ostream& out, const Record& rec)
			{
				out << rec.message;
			};
		}

		static auto defaultFilter() noexcept
		{
			return [](const Record& rec) { return true; };
		}

	public:
		BasicSink(std::ostream& stream) :
			m_Stream{ stream }
		{
			m_Formatter = defaultFormatter();
			m_Filter = defaultFilter();
		}

		~BasicSink() noexcept = default;

		BasicSink(const BasicSink&) = delete;
		BasicSink& operator =(const BasicSink&) = delete;

		BasicSink(BasicSink&&) = delete;
		BasicSink& operator =(BasicSink&&) = delete;

		void log(const Record& record) override
		{
			std::scoped_lock lock{ m_FilterMx, m_FormatterMx, m_WriteAccessMx };
			if (m_Filter(record))
			{
				m_Formatter(m_Stream, record);
				m_Stream << std::endl;
			}
		}

		template <RecordFormatter TFormatter>
		void setFormatter(TFormatter&& formatter) noexcept
		{
			std::scoped_lock lock{ m_FormatterMx };
			m_Formatter = std::forward<TFormatter>(formatter);
		}

		void removeFormatter() noexcept
		{
			std::scoped_lock lock{ m_FormatterMx };
			m_Formatter = defaultFormatter();
		}

		template <RecordFilter TFilter>
		void setFilter(TFilter&& filter) noexcept
		{
			std::scoped_lock lock{ m_FilterMx };
			m_Filter = std::forward<TFilter>(filter);
		}

		void removeFilter() noexcept
		{
			std::scoped_lock lock{ m_FilterMx };
			m_Filter = defaultFilter();
		}

	protected:
		std::mutex m_WriteAccessMx;

	private:
		std::ostream& m_Stream;

		std::mutex m_FormatterMx;
		Formatter_t m_Formatter;
		std::mutex m_FilterMx;
		Filter_t m_Filter;
	};
}

#endif
