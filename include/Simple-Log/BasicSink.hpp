//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_BASIC_SINK_HPP
#define SL_LOG_BASIC_SINK_HPP

#pragma once

#include <atomic>
#include <functional>
#include <iomanip>
#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>

#include "Concepts.hpp"
#include "FlushPolicies.hpp"
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
		using Formatter_t = std::function<std::string(const Record_t&)>;
		using Filter_t = std::function<bool(const Record_t&)>;
		using FlushPolicy_t = std::unique_ptr<detail::AbstractFlushPolicyWrapper<Record_t>>;

	protected:
		static constexpr Formatter_t defaultFormatter() noexcept
		{
			return [](const Record_t& rec)
			{
				using namespace std::chrono;
				using namespace std::chrono_literals;

				// ToDo: replace with c++20 chrono and format
				const auto today = rec.timePoint().time_since_epoch() % 24h;
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

				out << rec.severity() << ":: ";
				out << rec.message();
				return std::move(out).str();
			};
		}

		static constexpr Filter_t defaultFilter() noexcept
		{
			return [](const Record_t& rec) { return true; };
		}

		[[nodiscard]]
		static constexpr FlushPolicy_t defaultFlushPolicy() noexcept
		{
			return std::make_unique<detail::FlushPolicyWrapper<Record_t, AlwaysFlushPolicy>>();
		}

	public:
		/**
		 * \brief Constructor
		 * \param stream The stream object, which will receive finally formatted messages
		 */
		explicit BasicSink(std::ostream& stream) :
			m_Stream{ stream }
		{
		}

		/**
		 * \brief Default destructor
		 * \details Destructor does not perform any actions on the internal stream objects, due to it's potential dangling state. Derived classes must handle closing and flushing themselves.
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
		 * \brief Filters, formats and writes the passed record to the internal stream if this instance is enabled.
		 * \details This function prints the passed record to the internal stream object. In forehand, the active filter provides feedback if the record should be passed to the stream or not. If not, the call has no effect to any state.
		 *	The active formatter will be used to hand-over the necessary information of the Record object to the stream object.
		 * \param record Record object
		 */
		void log(const Record_t& record) final override
		{
			if (m_Enabled && logDerived(record))
			{
				if (std::scoped_lock lock{ m_FilterMx, m_FormatterMx, m_StreamMx }; std::invoke(m_Filter, record))
				{
					auto message = std::invoke(m_Formatter, record);
					auto size = std::size(message) * sizeof(typename decltype(message)::value_type);
					m_Stream << message;
					handleNewlineAndFlush(record, size);
				}
			}
		}

		/**
		 * \brief Enables or disables the Sink object
		 * \details Disabled Sinks will not handle any incoming Record s
		 * \param enable True will enable the Sink object.
		 */
		void enable(bool enable = true) noexcept final override
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

		/**
		 * \brief Sets the active Flush-Policy
		 * \tparam TPolicy   Type of the passed Flush-Policy (automatically deduced)
		 * \param policy The new Flush-Policy object
		 */
		template <FlushPolicyFor<Record_t> TPolicy>
		void setFlushPolicy(TPolicy&& policy) noexcept
		{
			std::scoped_lock lock{ m_FlushPolicyMx };
			m_FlushPolicy = std::make_unique<detail::FlushPolicyWrapper<TRecord, TPolicy>>(std::forward<TPolicy>(policy));
		}

		/**
		 * \brief Replaces the current Flush-Policy with the default one
		 * \details The default Flush-Policy flushes after each handled Record.
		 */
		void removeFlushPolicy() noexcept
		{
			std::scoped_lock lock{ m_FlushPolicyMx };
			m_FlushPolicy = defaultFlushPolicy();
		}

	protected:
		/**
		 * \brief Writes to the internal stream
		 * \details This functions writes directly to the stream object. No filter or formatter will be involved and stream will be flush afterwards.
		 * This might be useful for writing custom header or footer data to the stream.
		 * \tparam TData Type of data (automatically deduced)
		 * \param data Data which will be written to the stream.
		 */
		template <class TData>
		void writeToStream(TData&& data)
		{
			std::scoped_lock lock{ m_StreamMx };
			m_Stream << std::forward<TData>(data);
			flush();
		}

		/**
		 * \brief This function gets called before the actual writing.
		 * \details Subclasses may override this function if they want to perform any action before the actual logging process or apply custom filtering conditions (beside BasicSink s filter property).
		 * \param record The Record object.
		 * \return If false is returned, the Record will be skipped.
		 */
		virtual bool logDerived(const Record_t& record)
		{
			return true;
		}

	private:
		std::mutex m_StreamMx;
		std::ostream& m_Stream;

		std::mutex m_FormatterMx;
		Formatter_t m_Formatter{ defaultFormatter() };

		std::mutex m_FilterMx;
		Filter_t m_Filter{ defaultFilter() };

		std::mutex m_FlushPolicyMx;
		FlushPolicy_t m_FlushPolicy{ defaultFlushPolicy() };

		std::atomic_bool m_Enabled{ false };

		void handleNewlineAndFlush(const Record_t& record, std::size_t messageByteSize)
		{
			if (messageByteSize == 0)
				return;

			if (std::scoped_lock lock{ m_FlushPolicyMx }; std::invoke(*m_FlushPolicy, record, messageByteSize))
			{
				flush();
			}
			else
			{
				m_Stream << "\n";
			}
		}

		void flush()
		{
			m_Stream << std::endl;
			m_FlushPolicy->flushed();
		}
	};

	/** @}*/
}

#endif
