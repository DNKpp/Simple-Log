//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_ISINK_HPP
#define SL_LOG_ISINK_HPP

#pragma once

#include <stdexcept>

#include "Concepts.hpp"

namespace sl::log
{
	/** \addtogroup Sinks
	 * @{
	 */

	/**
	 * \brief Sink interface class
	 * \tparam TRecord Used Record type.
	 * \details The interface class for each derived Sink type. If you want to implement your custom Sink type, have a look at BasicSink first. This might be a better starting point for customizations.
	 */
	template <Record TRecord>
	class ISink
	{
	public:
		/**
		 * \brief Used Record type.
		 */
		using Record_t = TRecord;

		/**
		 * \brief Deleted copy-constructor
		 */
		ISink(const ISink&) = delete;
		/**
		 * \brief Deleted copy-assign operator
		 */
		ISink& operator =(const ISink&) = delete;

		/**
		 * \brief virtual default destructor
		 */
		virtual ~ISink() noexcept = default;

		/**
		 * \brief virtual log function
		 * \details Will be called from the Worker thread.
		 * \param record The record about to be processed by the sink
		 */
		virtual void log(const Record_t& record) = 0;

		/**
		 * \brief Enables or disables the Sink object
		 * \details Disabled Sinks will not handle any incoming Record s
		 * \param enable True will enable the Sink object.
		 */
		virtual void setEnabled(bool enable = true) noexcept = 0;

		/**
		 * \brief Checks if the Sink object is enabled.
		 * \return Returns true if object is enabled.
		 */
		[[nodiscard]]
		virtual bool isEnabled() const noexcept = 0;

	protected:
		/**
		 * \brief Default constructor
		 */
		ISink() = default;
		/**
		 * \brief Default move-constructor
		 */
		ISink(ISink&&) = default;
		/**
		 * \brief Default move-assign operator
		 */
		ISink& operator =(ISink&&) = default;
	};

	/**
	 * \brief Wrapper class which disables Sinks on construction and enables them on destruction
	 * \tparam TRecord Record type
	 * \tparam TSink Sink type
	 * \details This helper class is useful when you want to get sure, that your Sinks will be finally setup before they are going to handle any records.
	 * Instances of this class are movable but not copyable.
	 */
	template <Record TRecord, std::derived_from<ISink<TRecord>> TSink>
	class ScopedSinkDisabling
	{
	public:
		/**
		 * \brief Constructor which disables passed sink
		 * \param sink 
		 */
		ScopedSinkDisabling(TSink& sink) noexcept :
			m_Sink{ &sink }
		{
			m_Sink->setEnabled(false);
		}

		/**
		 * \brief Constructor which enables passed sink
		 */
		~ScopedSinkDisabling() noexcept
		{
			if (m_Sink)
			{
				m_Sink->setEnabled();
				m_Sink = nullptr;
			}
		}

		/**
		 * \brief Deleted copy constructor
		 */
		ScopedSinkDisabling(const ScopedSinkDisabling&) = delete;

		/**
		 * \brief Deleted copy-assign operator
		 */
		ScopedSinkDisabling& operator =(const ScopedSinkDisabling&) = delete;

		/**
		 * \brief Move constructor
		 */
		ScopedSinkDisabling(ScopedSinkDisabling&& other) noexcept
		{
			*this = std::move(other);
		}

		/**
		 * \brief Move-assign operator
		 */
		ScopedSinkDisabling& operator =(ScopedSinkDisabling&& other) noexcept
		{
			using std::swap;
			swap(m_Sink, other.m_Sink);
			return *this;
		}

		/**
		 * \brief Dereferencing operator
		 * \return Reference to the wrapped Sink instance.
		 */
		[[nodiscard]]
		TSink& operator *() const noexcept
		{
			return *m_Sink;
		}

		/**
		 * \brief Dereferencing operator
		 * \return Pointer to the wrapped Sink instance.
		 */
		[[nodiscard]]
		TSink* operator ->() const noexcept
		{
			return m_Sink;
		}

		/**
		 * \brief Returns a pointer to the Sink instance
		 * \return Pointer to the wrapped Sink instance.
		 */
		[[nodiscard]]
		TSink* get() const noexcept
		{
			return m_Sink;
		}

	private:
		TSink* m_Sink = nullptr;
	};

	class SinkException final :
		public std::runtime_error
	{
	public:
		explicit SinkException(const std::string& message) :
			runtime_error{ message }
		{
		}

		explicit SinkException(const char* message) :
			runtime_error{ message }
		{
		}
	};

	/** @}*/
}

#endif
