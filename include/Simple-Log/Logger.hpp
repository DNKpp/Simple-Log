//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_LOGGER_HPP
#define SL_LOG_LOGGER_HPP

#pragma once

#include <cassert>
#include <version>

#ifdef __cpp_lib_source_location
#include <source_location>
#endif

#include "Concepts.hpp"
#include "Core.hpp"
#include "RecordBuilder.hpp"

namespace sl::log
{
	/**
	 * \brief Convenience class for generating Record s
	 * \attention The corresponding Core instance must outlive all related Logger instances.
	 * \details This class is in fact a template for upcoming Record s. It stores default settings (e.g. severity level and channel) for newly generated Record s. Using its operator () users can start building Record s
	 * in an effective and elegant manner. Logger instances are rather lightweight, thus could be instantiated on class level, but it is also fine using the same instance in the whole program.
	 */
	template <Record TRecord>
	class Logger
	{
	public:
		using Record_t = TRecord;
		using SeverityLevel_t = typename Record_t::SeverityLevel_t;
		using Channel_t = typename Record_t::Channel_t;
		using UserData_t = typename Record_t::UserData_t;
		using Core_t = Core<Record_t>;
		using RecordBuilder_t = RecordBuilder<Record_t>;

		/**
		 * \brief Constructor
		 * \details Creates a Logger instance and links it to the specified core instance
		 * \param core Specific core instance
		 * \param defaultSeverityLvl Default severity level for generated Records.
		 * \param defaultChannel Default channel for generated Records.
		 */
		explicit Logger(
			Core_t& core,
			SeverityLevel_t defaultSeverityLvl = {},
			Channel_t defaultChannel = {}
		) noexcept :
			m_Core{ &core },
			m_DefaultSeverityLvl{ std::move(defaultSeverityLvl) },
			m_DefaultChannel{ std::move(defaultChannel) }
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
		 * \brief Copy-assign operator
		 */
		Logger& operator =(const Logger&) noexcept = default;

		/**
		 * \brief Move constructor
		 */
		Logger(Logger&&) noexcept = default;
		/**
		 * \brief Move-assign operator
		 */
		Logger& operator =(Logger&&) noexcept = default;

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
			assert(m_Core && "Pointer to core instance must be set.");
			Record_t prefabRec;
			prefabRec.setTimePoint(std::chrono::system_clock::now());
			prefabRec.setSeverity(m_DefaultSeverityLvl);
			prefabRec.setChannel(m_DefaultChannel);
			RecordBuilder_t builder{ std::move(prefabRec), [&core = *m_Core](Record_t rec) { core.log(std::move(rec)); } };

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
		void setDefaultSeverityLevel(USeverityLevel&& sevLvl) noexcept
		{
			m_DefaultSeverityLvl = std::forward<USeverityLevel>(sevLvl);
		}

		/**
		 * \brief Getter for the default severity level
		 * \return Returns a const reference to the default severity level.
		 */
		[[nodiscard]]
		const SeverityLevel_t& defaultSeverityLevel() const noexcept
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
		Core_t* m_Core = nullptr;
		SeverityLevel_t m_DefaultSeverityLvl;
		Channel_t m_DefaultChannel;
	};
}

#endif
