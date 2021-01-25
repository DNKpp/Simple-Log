//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_BASIC_SINK_HPP
#define SL_LOG_BASIC_SINK_HPP

#pragma once

#include <functional>
#include <iomanip>
#include <mutex>
#include <ostream>
#include <string>

#include "Concepts.hpp"
#include "ISink.hpp"

namespace sl::log
{
	/** \addtogroup sink
	 * @{
	 */

	/**
	 * \brief std::ostream orientated Sink class
	 * \details This Sink class uses a std::ostream reference for printing each recorded message and offers options to manipulate its behaviour: e.g. filtering and formatting messages. Due to the thread-safe design it is totally
	 *	fine changing settings during program runtime. 
	 *
	 *	This class offers everything you'll need to print messages into console via std::cout or std::cerr.
	 */
	class BasicSink :
		public ISink
	{
	protected:
		static auto defaultFormatter() noexcept
		{
			return [](std::ostream& out, const Record& rec)
			{
				using namespace std::chrono;

				const auto today = rec.time.time_since_epoch() % hours{ 24 };
				const auto hour = duration_cast<hours>(today);
				const auto minute = duration_cast<minutes>(today) % hours{ 1 };
				const auto second = duration_cast<seconds>(today) % minutes{ 1 };
				const auto millisecond = duration_cast<milliseconds>(today) % seconds{ 1 };
				out << std::setfill('0') <<
					std::setw(2) << hour.count() << ":" <<
					std::setw(2) << minute.count() << ":" <<
					std::setw(2) << second.count() << "." <<
					std::setw(3) << millisecond.count() <<
					" >>> ";

				if (auto* sevLvlPtr = std::any_cast<SeverityLevel>(&rec.severity))
				{
					out << *sevLvlPtr << "::";
				}
				out << rec.message;
			};
		}

		static auto defaultFilter() noexcept
		{
			return [](const Record& rec) { return true; };
		}

	public:
		/**
		 * \brief Constructor
		 * \param stream The stream object, which will be used for printing formatted messages
		 */
		BasicSink(std::ostream& stream) :
			m_Stream{ stream }
		{
			m_Formatter = defaultFormatter();
			m_Filter = defaultFilter();
		}

		/**
		 * \brief Default destructor
		 */
		~BasicSink() noexcept = default;

		/**
		 * \brief Deleted copy constructor
		 * \details
		 */
		BasicSink(const BasicSink&) = delete;
		/**
		 * \brief Deleted copy assign operator
		 * \details
		 */
		BasicSink& operator =(const BasicSink&) = delete;

		/**
		 * \brief Deleted move constructor
		 * \details
		 */
		BasicSink(BasicSink&&) = delete;
		/**
		 * \brief Deleted move assign operator
		 * \details
		 */
		BasicSink& operator =(BasicSink&&) = delete;

		/**
		 * \brief Filters, formats and writes the passed record to the internal stream
		 * \details This function prints the passed record to the internal stream object. In forehand, the active filter provides feedback if the record should be passed to the stream or not. If not, the call has no effect to any state.
		 *	The active formatter will be used to hand-over the necessary information of the Record object to the stream object.
		 * \param record Record object
		 */
		void log(const Record& record) override
		{
			std::scoped_lock lock{ m_FilterMx, m_FormatterMx, m_StreamMx };
			if (m_Filter(record))
			{
				m_Formatter(m_Stream, record);
				m_Stream << std::endl;
			}
		}

		/**
		 * \brief Sets the active formatter
		 * \details It's the formatters job to:
		 * \li extract the necessary information from the Record
		 * \li pass the extracted infos to the stream object
		 *
		 * This design decision is motivated by the fact, that it would be unnecessarily inefficient letting the formatter creating a temporary string object, which would simply get passed to the internal stream.
		 * Thus, a custom formatter also has to pass everything to the stream by itself.
		 *
		 * A formatter should use the following signature:
		 * \code{.cpp}
		 * void(std::ostream&, const Record&)
		 * \endcode
		 * \remark The formatters return type does not have to be void, but any returned information will be ignored.
		 *
		 * \tparam TFormatter Type of the passed formatter (automatically deduced)
		 * \param formatter An invokable formatter object
		 */
		template <RecordFormatter TFormatter>
		void setFormatter(TFormatter&& formatter) noexcept
		{
			std::scoped_lock lock{ m_FormatterMx };
			m_Formatter = std::forward<TFormatter>(formatter);
		}

		/**
		 * \brief Replaces the active formatter with the default one
		 */
		void removeFormatter() noexcept
		{
			std::scoped_lock lock{ m_FormatterMx };
			m_Formatter = defaultFormatter();
		}

		/**
		 * \brief Sets the active filter
		 * \details It's the filters job to decide, which Record will be printed (filter returns true) and which will be skipped (filter returns false). Therefore a filter must be an invokable of the following signature:
		 * \code{.cpp}
		 * bool(const Record&)
		 * \endcode
		 * \remark The filters return type doesn't have to be bool, but the returned object must at least be implicitly convertible to bool.
		 * 
		 * \tparam TFilter  Type of the passed filter (automatically deduced)
		 * \param filter  An invokable filter object
		 */
		template <RecordFilter TFilter>
		void setFilter(TFilter&& filter) noexcept
		{
			std::scoped_lock lock{ m_FilterMx };
			m_Filter = std::forward<TFilter>(filter);
		}

		/**
		 * \brief  Replaces the active filter with the default one
		 */
		void removeFilter() noexcept
		{
			std::scoped_lock lock{ m_FilterMx };
			m_Filter = defaultFilter();
		}

	protected:

		/**
		 * \brief Writes to the internal stream
		 * \details This functions writes directly to the stream object. No filter or formatter will be involved. This might be useful for writing custom header or footer data to the stream.
		 * \tparam TData Type of data (automatically deduced)
		 * \param data Data which will be written to the stream.
		 */
		template <class TData>
		void writeToStream(TData&& data)
		{
			std::scoped_lock lock{ m_StreamMx };
			m_Stream << std::forward<TData>(data);
		}

	private:
		using Formatter_t = std::function<void(std::ostream&, const Record&)>;
		using Filter_t = std::function<bool(const Record&)>;

		std::mutex m_StreamMx;
		std::ostream& m_Stream;

		std::mutex m_FormatterMx;
		Formatter_t m_Formatter;
		std::mutex m_FilterMx;
		Filter_t m_Filter;
	};

	/** @}*/
}

#endif
