//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_LOGGER_HPP
#define SL_LOG_LOGGER_HPP

#pragma once

#include <any>
#include <cassert>
#include <version>

#ifdef __cpp_lib_source_location
#include <source_location>
#endif

#include "Core.hpp"
#include "Record.hpp"
#include "RecordBuilder.hpp"

namespace sl::log
{
	/**
	 * \brief Convenience class for generating Record s
	 * \details This class is in fact a template for upcoming Record s. It stores default settings (e.g. severity level and channel) for newly generated Record s. Using its operator () users can start building Record s
	 * in an effective and elegant manner.
	 */
	class Logger
	{
	public:
		/**
		 * \brief Constructor
		 * \details Creates a Logger instance and links it to the specified core instance
		 * \param core Specific core instance
		 * \param defaultSeverityLvl Default severity level for generated Records.
		 * \param defaultChannel Default channel for generated Records.
		 * \param defaultUserData Default user data for generated Records.
		 */
		Logger(
			Core& core,
			std::any defaultSeverityLvl = {},
			std::any defaultChannel = {},
			std::any defaultUserData = {}
		) noexcept :
			m_Core{ &core },
			m_DefaultSeverityLvl{ std::move(defaultSeverityLvl) },
			m_DefaultChannel{ std::move(defaultChannel) },
			m_DefaultUserData{ std::move(defaultUserData) }
		{
		}

		/**
		 * \brief Destructor
		 */
		~Logger() noexcept = default;

		/**
		 * \brief Copy constructor
		 */
		Logger(const Logger&) noexcept = default;
		/**
		 * \brief Copy assign operator
		 */
		Logger& operator =(const Logger&) noexcept = default;

		/**
		 * \brief Move constructor
		 */
		Logger(Logger&&) noexcept = default;
		/**
		 * \brief Move assign operator
		 */
		Logger& operator =(Logger&&) noexcept = default;

		/**
		 * \brief Creates a new instance of RecordBuilder
		 * \details This is the entry point for producing new Record s. The returned RecordBuilder object will be pre-initialized with each default values,
		 * which might be overriden during the Record building process. 
		 * \return Newly created RecordBuilder instance.
		 */
#ifdef __cpp_lib_source_location
		RecordBuilder operator ()(const std::source_location& srcLoc = std::source_location::current())
#else
		RecordBuilder operator ()()
#endif
		{
			assert(m_Core);
			RecordBuilder builder{ [&core = *m_Core](Record rec) { core.log(std::move(rec)); } };
			builder.record().severity = m_DefaultSeverityLvl;
			builder.record().channel = m_DefaultChannel;
			builder.record().userData = m_DefaultUserData;

#ifdef __cpp_lib_source_location
			builder.record().sourceLocation = srcLoc;
#endif

			return builder;
		}

		/**
		 * \brief Setter for the default severity level
		 * \param sevLvl New default value
		 */
		void setDefaultSeverityLevel(std::any sevLvl) noexcept
		{
			m_DefaultSeverityLvl = std::move(sevLvl);
		}

		/**
		 * \brief Getter for the default severity level
		 * \return Returns a const reference to the default severity level.
		 */
		[[nodiscard]] const std::any& defaultSeverityLevel() const noexcept
		{
			return m_DefaultSeverityLvl;
		}

		/**
		 * \brief Setter for the default channel
		 * \param channel New default value
		 */
		void setDefaultChannel(std::any channel) noexcept
		{
			m_DefaultChannel = std::move(channel);
		}

		/**
		 * \brief Getter for the default channel
		 * \return Returns a const reference to the default channel.
		 */
		[[nodiscard]] const std::any& defaultChannel() const noexcept
		{
			return m_DefaultChannel;
		}

		/**
		 * \brief Setter for the default user data
		 * \param userData New default value
		 */
		void setDefaultUserData(std::any userData) noexcept
		{
			m_DefaultUserData = std::move(userData);
		}

		/**
		 * \brief Getter for the default user data
		 * \return Returns a const reference to the default user data.
		 */
		[[nodiscard]] const std::any& defaultUserData() const noexcept
		{
			return m_DefaultUserData;
		}

	private:
		Core* m_Core = nullptr;
		std::any m_DefaultSeverityLvl;
		std::any m_DefaultChannel;
		std::any m_DefaultUserData;
	};
}

#endif
