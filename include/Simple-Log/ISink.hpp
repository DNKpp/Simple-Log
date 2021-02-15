//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_ISINK_HPP
#define SL_LOG_ISINK_HPP

#pragma once

#include "Concepts.hpp"

namespace sl::log
{
	/** \addtogroup sink
	 * @{
	 */

	/**
	 * \brief Sink interface class
	 * \details The interface class for each derived Sink type. If you want to implement your custom Sink type, have a look at BasicSink first. This might be a better starting point for customizations.
	 */
	template <Record TRecord>
	class ISink
	{
	public:
		using Record_t = TRecord;

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
	};

	/** @}*/
}

#endif
