//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_OSTREAM_SINK_HPP
#define SL_LOG_OSTREAM_SINK_HPP

#pragma once

#include <memory>
#include <ostream>
#include <mutex>
#include <string_view>

#include "Concepts.hpp"
#include "BasicSink.hpp"
#include "FlushPolicies.hpp"

namespace sl::log
{
	/** \addtogroup Sinks
	 * @{
	 */

	/**
	 * \brief An std::ostream orientated Sink class which extends BasicSink
	 * \tparam TRecord Used Record type.
	 * \details This Sink class uses a std::ostream reference for printing each recorded message and offers options to manipulate its behaviour: e.g. filtering and formatting messages.
	 *	Users which would like to print messages onto the console, there is already an existing class ConsoleSink. For file related logging FileSink might be more suitable.
	 */
	template <Record TRecord>
	class OStreamSink :
		public BasicSink<TRecord>
	{
		using Super = BasicSink<TRecord>;

	public:
		using typename Super::Record_t;
		using typename Super::Formatter_t;
		using typename Super::Filter_t;
		using FlushPolicy_t = std::unique_ptr<detail::AbstractFlushPolicyWrapper<Record_t>>;

	protected:
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
		explicit OStreamSink(std::ostream& stream) :
			Super{},
			m_Stream{ stream }
		{
		}

		/**
		 * \brief Default destructor
		 * \details Destructor does not perform any actions on the internal stream objects, due to it's potential dangling state. Derived classes must handle closing and flushing themselves.
		 */
		~OStreamSink() noexcept = default;

		/**
		 * \brief Deleted copy constructor
		 */
		OStreamSink(const OStreamSink&) = delete;
		/**
		 * \brief Deleted copy assign operator
		 */
		OStreamSink& operator =(const OStreamSink&) = delete;

		/**
		 * \brief Deleted move constructor
		 */
		OStreamSink(OStreamSink&&) = delete;
		/**
		 * \brief Deleted move assign operator
		 */
		OStreamSink& operator =(OStreamSink&&) = delete;

		/**
		 * \brief Sets the active Flush-Policy
		 * \tparam TPolicy Type of the passed Flush-Policy (automatically deduced)
		 * \param policy The new Flush-Policy object
		 */
		template <FlushPolicyFor<Record_t> TPolicy>
		void setFlushPolicy(TPolicy&& policy)
		{
			std::scoped_lock lock{ m_FlushPolicyMx };
			m_FlushPolicy = std::make_unique<detail::FlushPolicyWrapper<TRecord, TPolicy>>(std::forward<TPolicy>(policy));
		}

		/**
		 * \brief Replaces the current Flush-Policy with the default one
		 * \details The default Flush-Policy flushes after each handled Record.
		 */
		void removeFlushPolicy()
		{
			std::scoped_lock lock{ m_FlushPolicyMx };
			m_FlushPolicy = defaultFlushPolicy();
		}

		/**
		 * \brief Flushes all pending output of the internal stream
		 * \remark Internally locks the associated stream mutex.
		 */
		void flush()
		{
			std::scoped_lock lock{ m_StreamMx };
			flushImpl();
		}

	protected:
		/**
		 * \brief Writes directly to the internal stream
		 * \tparam TData Type of data (automatically deduced)
		 * \param data Data which will be written to the stream.
		 * 
		 * \details This functions writes directly to the stream object. No filter or formatter will be involved and stream will be flush afterwards.
		 * This might be useful for writing custom header or footer data to the stream.
		 * \remark If not already locked, this function will lock the stream associated mutex.
		 */
		template <class TData>
		void writeToStream(TData&& data)
		{
			std::scoped_lock lock{ m_StreamMx };
			m_Stream << std::forward<TData>(data);
			flushImpl();
		}

		/**
		 * \brief Virtual method which will be called before the actual message is written to the stream
		 * \param record The current handled Record object
		 * \param message The final message
		 * \remark The stream associated mutex is locked before this function gets invoked.
		 * \version since alpha-0.6
		 */
		virtual void beforeMessageWrite(const Record_t& record, std::string_view message)
		{
		}

		/**
		 * \brief Virtual method which will be called after the actual message is written to the stream
		 * \param record The current handled Record object
		 * \param message The final message
		 * \remark The stream associated mutex is locked before this function gets invoked.
		 * \version since alpha-0.6
		 */
		virtual void afterMessageWrite(const Record_t& record, std::string_view message)
		{
		}

	private:
		std::recursive_mutex m_StreamMx;
		std::ostream& m_Stream;

		std::mutex m_FlushPolicyMx;
		FlushPolicy_t m_FlushPolicy{ defaultFlushPolicy() };

		void handleFlushPolicy(const Record_t& record, std::size_t messageByteSize)
		{
			if (std::scoped_lock lock{ m_FlushPolicyMx }; !std::invoke(*m_FlushPolicy, record, messageByteSize))
				return;

			flushImpl();
		}

		void flushImpl()
		{
			m_Stream << std::flush;
			m_FlushPolicy->flushed();
		}

		void writeMessage(const Record_t& record, std::string_view message) final override
		{
			const auto msgSize = std::size(message) * sizeof(std::string_view::value_type);

			std::scoped_lock lock{ m_StreamMx };
			beforeMessageWrite(record, message);
			m_Stream << message << "\n";
			afterMessageWrite(record, message);
			handleFlushPolicy(record, msgSize);
		}
	};

	/** @}*/
}

#endif
