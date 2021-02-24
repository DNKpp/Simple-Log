//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_CONSOLE_SINK_HPP
#define SL_LOG_CONSOLE_SINK_HPP

#pragma once

#include "Concepts.hpp"
#include "OStreamSink.hpp"
#include "third_party/rang/rang.hpp"

#include <iostream>
#include <mutex>
#include <functional>
#include <cassert>
#include <variant>
#include <unordered_map>

namespace sl::log
{
	/** \addtogroup Sinks
	 * @{
	 */
	
	/** \addtogroup ConsoleSink
	 * @{
	 */
	
	struct ConsoleTextStyle
	{
		using Style = rang::style;
		using TextColor = rang::fg;
		using TextColorBright = rang::fgB;
		using BgColor = rang::bg;
		using BgColorBright = rang::bgB;

		Style style = Style::reset;
		std::variant<TextColor, TextColorBright> textColor = TextColor::reset;
		std::variant<BgColor, BgColorBright> bgColor = BgColor::reset;
	};

	constexpr ConsoleTextStyle defaultConsoleTextStyle;

	inline std::ostream& operator <<(std::ostream& out, const ConsoleTextStyle& style)
	{
		auto putColor = [&out](auto color) { out << color; };
		out << style.style;
		std::visit(putColor, style.textColor);
		std::visit(putColor, style.bgColor);
		return out;
	}

	template <class TProjection, class TTable>
	class ConsoleTextStyleTable
	{
	public:
		using Projection_t = std::remove_cvref_t<TProjection>;
		using Table_t = std::remove_cvref_t<TTable>;

		ConsoleTextStyleTable(TProjection projection, TTable table) :
			m_Projection{ std::move(projection) },
			m_StyleTable{ std::move(table) }
		{
		}

		template <Record TRecord>
		ConsoleTextStyle operator ()(const TRecord& record) const
		{
			if (auto itr = m_StyleTable.find(std::invoke(m_Projection, record)); itr != std::end(m_StyleTable))
			{
				return itr->second;
			}
			return defaultConsoleTextStyle;
		}

	private:
		Projection_t m_Projection;
		Table_t m_StyleTable;
	};

	template <Record TRecord,
		std::invocable<const TRecord&> TProjection,
		class TTable = std::unordered_map<
			std::invoke_result_t<TProjection, const TRecord&>,
			ConsoleTextStyle
		>>
	auto makeConsoleTextStyleTableFor(TProjection projection, TTable table)
	{
		return ConsoleTextStyle{ std::move(projection), std::move(table) };
	}

	/**
	 * \brief std::ostream orientated Sink class
	 * \tparam TRecord Used Record type.
	 * \details This Sink class uses a std::ostream reference for printing each recorded message and offers options to manipulate its behaviour: e.g. filtering and formatting messages. Due to the thread-safe design it is totally
	 *	fine changing settings during program runtime. 
	 *
	 *	This class offers everything you'll need to print messages into console via std::cout, std::cerr or any other std::ostream object. For file related logging FileSink might be more suitable.
	 */
	template <Record TRecord>
	class ConsoleSink final :
		public OStreamSink<TRecord>
	{
		using Super = OStreamSink<TRecord>;

	public:
		using typename Super::Record_t;
		using typename Super::Formatter_t;
		using typename Super::Filter_t;
		using typename Super::FlushPolicy_t;
		using TextStylePolicy_t = std::function<ConsoleTextStyle(const Record_t&)>;

	protected:
		[[nodiscard]]
		static constexpr TextStylePolicy_t defaultTextStylePolicy()
		{
			return [](const Record_t&) { return defaultConsoleTextStyle; };
		}

	public:
		/**
		 * \brief Constructor
		 */
		explicit ConsoleSink() :
			Super{ std::cout }
		{
		}

		/**
		 * \brief Default destructor
		 * \details Destructor does not perform any actions on the internal stream objects, due to it's potential dangling state. Derived classes must handle closing and flushing themselves.
		 */
		~ConsoleSink() noexcept = default;

		/**
		 * \brief Deleted copy constructor
		 */
		ConsoleSink(const ConsoleSink&) = delete;
		/**
		 * \brief Deleted copy assign operator
		 */
		ConsoleSink& operator =(const ConsoleSink&) = delete;

		/**
		 * \brief Deleted move constructor
		 */
		ConsoleSink(ConsoleSink&&) = delete;
		/**
		 * \brief Deleted move assign operator
		 */
		ConsoleSink& operator =(ConsoleSink&&) = delete;

		template <ConsoleTextStylePolicyFor<Record_t> TStylePolicy>
		void setTextStylePolicy(TStylePolicy&& policy)
		{
			std::scoped_lock lock{ m_TextStylePolicyMx };
			m_TextStylePolicy = std::forward<TStylePolicy>(policy);
		}

		void removeTextStylePolicy()
		{
			std::scoped_lock lock{ m_TextStylePolicyMx };
			m_TextStylePolicy = defaultTextStylePolicy();
		}

	private:
		std::mutex m_TextStylePolicyMx;
		TextStylePolicy_t m_TextStylePolicy{ defaultTextStylePolicy() };

		void writeBeforeMessage(std::ostream& out, const Record_t& record) override
		{
			std::scoped_lock lock{ m_TextStylePolicyMx };
			assert(m_TextStylePolicy && "TextStylePolicy must not be empty.");
			out << std::invoke(m_TextStylePolicy, record);
		}

		void writeAfterMessage(std::ostream& out, const Record_t& record) override
		{
			out << defaultConsoleTextStyle;
		}
	};

	/** @}*/
	/** @}*/
}

#endif
