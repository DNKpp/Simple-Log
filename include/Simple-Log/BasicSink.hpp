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

#include "Concepts.hpp"
#include "ISink.hpp"

namespace sl::log
{
	/** \addtogroup sink
	 * @{
	 */

	/**
	 * \brief std::ostream orientated Sink class
	 * \tparam TRecord Used Record type.
	 * \details This Sink class uses a std::ostream reference for printing each recorded message and offers options to manipulate its behaviour: e.g. filtering and formatting messages. Due to the thread-safe design it is totally
	 *	fine changing settings during program runtime. 
	 *
	 *	This class offers everything you'll need to print messages into console via std::cout, std::cerr or any other std::ostream object. For file related logging FileSink might be more suitable.
	 */
	template <Record TRecord>
	class BasicSink :
		public ISink<TRecord>
	{
		using Super = ISink<TRecord>;

	public:
		using typename Super::Record_t;
		using Formatter_t = std::function<void(std::ostream&, const Record_t&)>;
		using Filter_t = std::function<bool(const Record_t&)>;

	protected:
		static auto defaultFormatter() noexcept
		{
			return [](std::ostream& out, const Record_t& rec)
			{
				using namespace std::chrono;
				using namespace std::chrono_literals;

				// ToDo: replace with c++20 chrono and format
				const auto today = rec.timePoint().time_since_epoch() % 24h;
				const auto hour = duration_cast<hours>(today);
				const auto minute = duration_cast<minutes>(today) % 1h;
				const auto second = duration_cast<seconds>(today) % 1min;
				const auto millisecond = duration_cast<milliseconds>(today) % 1s;
				out << std::setfill('0') <<
					std::setw(2) << hour.count() << ":" <<
					std::setw(2) << minute.count() << ":" <<
					std::setw(2) << second.count() << "." <<
					std::setw(3) << millisecond.count() <<
					" >>> ";

				out << rec.severity() << ":: ";
				out << rec.message();
			};
		}

		static auto defaultFilter() noexcept
		{
			return [](const Record_t& rec) { return true; };
		}

	public:
		/**
		 * \brief Constructor
		 * \param stream The stream object, which will receive finally formatted messages
		 */
		explicit BasicSink(std::ostream& stream) :
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
		 */
		BasicSink(const BasicSink&) = delete;
		/**
		 * \brief Deleted copy assign operator
		 */
		BasicSink& operator =(const BasicSink&) = delete;

		/**
		 * \brief Deleted move constructor
		 */
		BasicSink(BasicSink&&) = delete;
		/**
		 * \brief Deleted move assign operator
		 */
		BasicSink& operator =(BasicSink&&) = delete;

		/**
		 * \brief Filters, formats and writes the passed record to the internal stream
		 * \details This function prints the passed record to the internal stream object. In forehand, the active filter provides feedback if the record should be passed to the stream or not. If not, the call has no effect to any state.
		 *	The active formatter will be used to hand-over the necessary information of the Record object to the stream object.
		 * \param record Record object
		 */
		void log(const Record_t& record) override
		{
			std::scoped_lock lock{ m_FilterMx, m_FormatterMx, m_StreamMx };
			if (std::invoke(m_Filter, record))
			{
				std::invoke(m_Formatter, m_Stream, record);
				m_Stream << std::endl;
			}
		}

		/**
		 * \brief Sets the active formatter
		 * \details It's the formatters job to:
		 * \li extract the necessary information from Records
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
		template <RecordFormatterFor<Record_t> TFormatter>
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
		template <RecordFilterFor<Record_t> TFilter>
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
