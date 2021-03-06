//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_BASIC_SINK_HPP
#define SL_LOG_BASIC_SINK_HPP

#pragma once

#include "ISink.hpp"
#include "Record.hpp"

#include <atomic>
#include <chrono>
#include <functional>
#include <iomanip>
#include <mutex>
#include <sstream>

namespace sl::log
{
	/** \addtogroup Sinks
	 * @{
	 */

	/**
	 * \brief Concept for invokable formatter objects
	*/
	template <class T, class TRecord>
	concept RecordFormatterFor =
		Record<TRecord> &&
		std::is_invocable_r_v<std::string, T, const TRecord&>;

	/**
	 * \brief Concept for invokable filter objects
	*/
	template <class T, class TRecord>
	concept RecordFilterFor =
		Record<TRecord> &&
		std::predicate<T, const TRecord&>;

	/**
	 * \brief Abstract Sink class which offers basic filtering, formatting functionality
	 * \tparam TRecord Used Record type.
	 * \details This Sink class implements the enabling functionality, as well as filtering and formatting Records. Users who want to print messages into a std::ostream like object, should
	 * look at the OStreamSink or its derived classes.
	 */
	template <Record TRecord>
	class BasicSink :
		public ISink<TRecord>
	{
		using Super = ISink<TRecord>;

	public:
		using typename Super::Record_t;
		using Projections_t = RecordGetters<Record_t>;
		using Formatter_t = std::function<std::string(const Record_t&)>;
		using Filter_t = std::function<bool(const Record_t&)>;

	protected:
		static constexpr Formatter_t defaultFormatter() noexcept
		{
			return [](const Record_t& rec)
			{
				using namespace std::chrono;
				using namespace std::chrono_literals;

				// ToDo: replace with c++20 chrono and format
				const auto today = Projections_t::timePoint(rec).time_since_epoch() % 24h;
				const auto hour = duration_cast<hours>(today);
				const auto minute = duration_cast<minutes>(today) % 1h;
				const auto second = duration_cast<seconds>(today) % 1min;
				const auto millisecond = duration_cast<milliseconds>(today) % 1s;

				std::ostringstream out;
				out << std::setfill('0') <<
					std::setw(2) << hour.count() << ":" <<
					std::setw(2) << minute.count() << ":" <<
					std::setw(2) << second.count() << "." <<
					std::setw(3) << millisecond.count() <<
					" >>> ";

				out << Projections_t::severity(rec) << ":: ";
				out << Projections_t::message(rec);
				return std::move(out).str();
			};
		}

		static constexpr Filter_t defaultFilter() noexcept
		{
			return [](const Record_t& rec) { return true; };
		}

	public:
		/**
		 * \brief Constructor
		 */
		explicit BasicSink() noexcept = default;

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
		 * \brief Handles the given Record object
		 * \details Before the Record gets passed to the actual destination, it will be checked if the Sink object is enabled and if the Record should be filtered.
		 * If these checks are passed, the abstract writeMessage function will be invoked with the finally formatted message string.
		 * \param record Record object
		 */
		void log(const Record_t& record) final override
		{
			if (!m_Enabled)
				return;

			if (std::scoped_lock lock{ m_FilterMx }; !std::invoke(m_Filter, record))
				return;

			const auto message = [&]
			{
				std::scoped_lock lock{ m_FormatterMx };
				return std::invoke(m_Formatter, record);
			}();

			writeMessage(record, message);
		}

		/**
		 * \brief Enables or disables the Sink object
		 * \details Disabled Sinks will not handle any incoming Record s
		 * \param enable True will enable the Sink object.
		 */
		void setEnabled(bool enable = true) noexcept final override
		{
			m_Enabled = enable;
		}

		/**
		 * \brief Checks if the Sink object is enabled.
		 * \return Returns true if object is enabled.
		 */
		[[nodiscard]]
		bool isEnabled() const noexcept final override
		{
			return m_Enabled;
		}

		/**
		 * \brief Sets the active formatter
		 * \details It's the formatters job to extract the necessary information from Records and built the final message string.
		 *
		 * A custom formatter should use the following signature:
		 * \code{.cpp}
		 * std::string(const Record&)
		 * \endcode
		 *
		 * \tparam TFormatter Type of the passed formatter (automatically deduced)
		 * \param formatter An invokable formatter object
		 */
		template <RecordFormatterFor<Record_t> TFormatter>
		void setFormatter(TFormatter&& formatter)
		{
			std::scoped_lock lock{ m_FormatterMx };
			m_Formatter = std::forward<TFormatter>(formatter);
		}

		/**
		 * \brief Replaces the active formatter with the default one
		 */
		void removeFormatter()
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
		void setFilter(TFilter&& filter)
		{
			std::scoped_lock lock{ m_FilterMx };
			m_Filter = std::forward<TFilter>(filter);
		}

		/**
		 * \brief  Replaces the active filter with the default one
		 */
		void removeFilter()
		{
			std::scoped_lock lock{ m_FilterMx };
			m_Filter = defaultFilter();
		}

	protected:
		/**
		 * \brief This function will be called when the actual message should be printed.
		 * \details Subclasses must implement this function and have to print the message to the desired destination themselves.
		 * \param record The Record object.
		 * \param message The actual message, which should be printed.
		 * \version since alpha-0.6
		 */
		virtual void writeMessage(const Record_t& record, std::string_view message) = 0;

	private:
		std::mutex m_FormatterMx;
		Formatter_t m_Formatter{ defaultFormatter() };

		std::mutex m_FilterMx;
		Filter_t m_Filter{ defaultFilter() };

		std::atomic_bool m_Enabled{ false };
	};

	/** @}*/
}

#endif
