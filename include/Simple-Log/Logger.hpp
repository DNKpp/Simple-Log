//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_LOGGER_HPP
#define SL_LOG_LOGGER_HPP

#pragma once

#include <cassert>
#include <functional>
#include <version>

#ifdef __cpp_lib_source_location
#include <source_location>
#endif

#include "Concepts.hpp"
#include "Core.hpp"
#include "RecordBuilder.hpp"

namespace sl::log
{
	/** \addtogroup Logger
	 * @{
	 */

	/**
	 * \brief Convenience class for generating Record s
	 * \tparam TRecord Used Record type.
	 * \attention The corresponding Core instance must outlive all related Logger instances.
	 * \details This class is in fact a template for upcoming Record s. It stores default settings (e.g. severity level and channel) for newly generated Record s. Using its operator () users can start building Record s
	 * in an effective and elegant manner. Logger instances are rather lightweight, thus could be instantiated on class level, but it is also fine using the same instance in the whole program.
	 */
	template <Record TRecord>
	class BaseLogger
	{
	public:
		using Record_t = TRecord;
		using SeverityLevel_t = typename Record_t::SeverityLevel_t;
		using Channel_t = typename Record_t::Channel_t;
		using RecordBuilder_t = RecordBuilder<Record_t>;
		using LogCallback_t = std::function<void(Record_t)>;

		/**
		 * \brief Constructor
		 * \details Creates a Logger instance and links it to the specified core instance
		 * \param logCallback Callback which will receive created Records.
		 * \param defaultSeverityLvl Default severity level for generated Records.
		 * \param defaultChannel Default channel for generated Records.
		 */
		explicit BaseLogger(
			LogCallback_t logCallback,
			SeverityLevel_t defaultSeverityLvl = {},
			Channel_t defaultChannel = {}
		) noexcept :
			m_LogCallback{ std::move(logCallback) },
			m_DefaultSeverityLvl{ std::move(defaultSeverityLvl) },
			m_DefaultChannel{ std::move(defaultChannel) }
		{
		}

		/**
		 * \brief Destructor
		 */
		~BaseLogger() noexcept = default;

		/**
		 * \brief Copy constructor
		 */
		BaseLogger(const BaseLogger&) noexcept = default;
		/**
		 * \brief Copy-assign operator
		 */
		BaseLogger& operator =(const BaseLogger&) noexcept = default;

		/**
		 * \brief Move constructor
		 */
		BaseLogger(BaseLogger&&) noexcept = default;
		/**
		 * \brief Move-assign operator
		 */
		BaseLogger& operator =(BaseLogger&&) noexcept = default;

		/**
		 * \brief Creates a new instance of RecordBuilder
		 * \details This is the entry point for producing new Record s. The returned RecordBuilder object will be pre-initialized with each values set to Logger's default,
		 * which might be overriden during the Record building process. 
		 * \return Newly created RecordBuilder instance.
		 */
		[[nodiscard]]
#ifdef __cpp_lib_source_location
		RecordBuilder_t operator ()(const std::source_location& srcLoc = std::source_location::current())
#else
		RecordBuilder_t operator ()()
#endif
		{
			assert(m_LogCallback != nullptr && "Log callback must be set.");
			Record_t prefabRec;
			prefabRec.setTimePoint(std::chrono::system_clock::now());
			prefabRec.setSeverity(m_DefaultSeverityLvl);
			prefabRec.setChannel(m_DefaultChannel);
			RecordBuilder_t builder{ std::move(prefabRec), m_LogCallback };

#ifdef __cpp_lib_source_location
			builder.record().sourceLocation = srcLoc;
#endif

			return builder;
		}

		/**
		 * \brief Setter for the default severity level
		 * \param sevLvl New default value
		 */
		template <std::convertible_to<SeverityLevel_t> USeverityLevel>
		void setDefaultSeverity(USeverityLevel&& sevLvl) noexcept
		{
			m_DefaultSeverityLvl = std::forward<USeverityLevel>(sevLvl);
		}

		/**
		 * \brief Getter for the default severity level
		 * \return Returns a const reference to the default severity level.
		 */
		[[nodiscard]]
		const SeverityLevel_t& defaultSeverity() const noexcept
		{
			return m_DefaultSeverityLvl;
		}

		/**
		 * \brief Setter for the default channel
		 * \param channel New default value
		 */
		template <std::convertible_to<Channel_t> UChannel>
		void setDefaultChannel(UChannel&& channel) noexcept
		{
			m_DefaultChannel = std::forward<UChannel>(channel);
		}

		/**
		 * \brief Getter for the default channel
		 * \return Returns a const reference to the default channel.
		 */
		[[nodiscard]]
		const Channel_t& defaultChannel() const noexcept
		{
			return m_DefaultChannel;
		}

	private:
		LogCallback_t m_LogCallback;
		SeverityLevel_t m_DefaultSeverityLvl;
		Channel_t m_DefaultChannel;
	};

	/**
	 * \brief Creates a Logger object and setup its callback to the given Core instance
	 * \relates BaseLogger
	 * \tparam TLogger Concrete Logger type
	 * \tparam TArgs Constructor argument types (will be deducted automatically)
	 * \param core The core instance the Logger object should contribute to.
	 * \param args The constructor arguments for the newly generated Logger object. Will be forwarded as is.
	 * \return Logger object
	 * \details This function creates a new Logger object and returns it to the caller. This Logger will receive a callback to the given Core instance, but Core does not
	 * take over ownership of the created Logger object. If users does not need the Logger object any longer, they may simply let them go out of scope.
	 */
	template <Logger TLogger, class... TArgs>
	TLogger makeLogger(Core<typename TLogger::Record_t>& core, TArgs&&... args)
	{
		return TLogger{
			[&core](auto rec)
			{
				core.log(std::move(rec));
			},
			std::forward<TArgs>(args)...
		};
	}

	/** @}*/
}

#endif
