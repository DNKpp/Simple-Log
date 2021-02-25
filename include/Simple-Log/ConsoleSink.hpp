//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_CONSOLE_SINK_HPP
#define SL_LOG_CONSOLE_SINK_HPP

#pragma once

#include "Concepts.hpp"
#include "OStreamSink.hpp"

#define NOMINMAX
#include "third_party/rang/rang.hpp"

#include <cassert>
#include <functional>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <variant>

namespace sl::log
{
	/** \addtogroup Sinks
	 * @{
	 */

	/** \addtogroup ConsoleSink
	 * @{
	 */

	/**
	 * \brief Collection of possible style and color options for text printed onto the console
	 * \details This struct simply uses the types provided by the third-party lib rang. Some options might not work on every console.
	 * Go to https://github.com/agauniyal/rang/tree/master if you are interested about all the details.
	 * 
	 */
	struct ConsoleTextStyle
	{
		/**
		 * \brief Manipulates the font style
		 * \details Determines the appearance of the printed text:
		 * Code						|Linux/Win/Others	|	Old Win
		 * -------------------------|-------------------|-------------
		 * Style::bold				|	yes				|	yes
		 * Style::dim				|	yes				|	no
		 * Style::italic			|	yes				|	no
		 * Style::underline			|	yes				|	no
		 * Style::blink				|	no				|	no
		 * Style::rblink			|	no				|	no
		 * Style::reversed			|	yes				|	yes
		 * Style::conceal			|	maybe			|	yes
		 * Style::crossed			|	yes				|	no
		 * Style::reset				|	yes				|	no
		 */
		using Style = rang::style;

		/**
		 * \brief Manipulates the font color
		 * \details Determines the color of the printed text:
		 * Code						|Linux/Win/Others	|	Old Win
		 * -------------------------|-------------------|-------------
		 * TextColor::black			|	yes				|	yes
		 * TextColor::red			|	yes				|	yes
		 * TextColor::green			|	yes				|	yes
		 * TextColor::yellow		|	yes				|	yes
		 * TextColor::blue			|	yes				|	yes
		 * TextColor::magenta		|	yes				|	yes
		 * TextColor::cyan			|	yes				|	yes
		 * TextColor::gray			|	yes				|	yes
		 * TextColor::reset			|	yes				|	yes
		 */
		using TextColor = rang::fg;
		
		/**
		 * \brief Manipulates the font color (slightly different tone than TextColor)
		 * \details Determines the color of the printed text:
		 * Code						|Linux/Win/Others	|	Old Win
		 * -------------------------|-------------------|-------------
		 * TextColorBright::black	|	yes				|	yes
		 * TextColorBright::red		|	yes				|	yes
		 * TextColorBright::green	|	yes				|	yes
		 * TextColorBright::yellow	|	yes				|	yes
		 * TextColorBright::blue	|	yes				|	yes
		 * TextColorBright::magenta	|	yes				|	yes
		 * TextColorBright::cyan	|	yes				|	yes
		 * TextColorBright::gray	|	yes				|	yes
		 */
		using TextColorBright = rang::fgB;

		/**
		 * \brief Manipulates the background color
		 * \details Determines the background color of the printed text:
		 * Code						|Linux/Win/Others	|	Old Win
		 * -------------------------|-------------------|-------------
		 * BgColor::black			|	yes				|	yes
		 * BgColor::red				|	yes				|	yes
		 * BgColor::green			|	yes				|	yes
		 * BgColor::yellow			|	yes				|	yes
		 * BgColor::blue			|	yes				|	yes
		 * BgColor::magenta			|	yes				|	yes
		 * BgColor::cyan			|	yes				|	yes
		 * BgColor::gray			|	yes				|	yes
		 * BgColor::reset			|	yes				|	yes
		 */
		using BgColor = rang::bg;

		/**
		 * \brief Manipulates the font color (slightly different tone than TextColor)
		 * \details Determines the color of the printed text:
		 * Code						|Linux/Win/Others	|	Old Win
		 * -------------------------|-------------------|-------------
		 * BgColorBright::black		|	yes				|	yes
		 * BgColorBright::red		|	yes				|	yes
		 * BgColorBright::green		|	yes				|	yes
		 * BgColorBright::yellow	|	yes				|	yes
		 * BgColorBright::blue		|	yes				|	yes
		 * BgColorBright::magenta	|	yes				|	yes
		 * BgColorBright::cyan		|	yes				|	yes
		 * BgColorBright::gray		|	yes				|	yes
		 */
		using BgColorBright = rang::bgB;

		/**
		 * \brief determines the style
		 */
		Style style = Style::reset;

		/**
		 * \brief determines the text color
		 */
		std::variant<TextColor, TextColorBright> textColor = TextColor::reset;

		/**
		 * \brief determines the background color
		 */
		std::variant<BgColor, BgColorBright> bgColor = BgColor::reset;
	};

	/**
	 * \brief A constant object used for resetting the style back to default
	 */
	constexpr ConsoleTextStyle defaultConsoleTextStyle;

	/**
	 * \brief Operator << overload for ConsoleTextStyle type
	 * \param out the stream object
	 * \param style the provided style
	 * \return Returns the parameter out as reference
	 */
	inline std::ostream& operator <<(std::ostream& out, const ConsoleTextStyle& style)
	{
		auto putColor = [&out](auto color) { out << color; };
		out << style.style;
		std::visit(putColor, style.textColor);
		std::visit(putColor, style.bgColor);
		return out;
	}

	/**
	 * \brief Convenience class for setting up style policies for a given Record member
	 * \tparam TProjection The type of the projection
	 * \tparam TTable The type of the internally used container. Can be either std::map or std::unordered_map
	 * \details Users have to provide a finally established container, which will then used for determining the appearance of the current Record.
	 * If no matching entry is found, defaultConsoleTextStyle will be used instead.
	 *
	 * Instead of directly constructing instances of this class, users should use the makeConsoleTextStyleTableFor function.
	 */
	template <class TProjection, class TTable>
	class ConsoleTextStyleTable
	{
	public:
		using Projection_t = std::remove_cvref_t<TProjection>;
		using Table_t = std::remove_cvref_t<TTable>;

		/**
		 * \brief Constructor
		 * \param projection The projection to a member of the currently used Record type
		 * \param table The finally established container object
		 */
		ConsoleTextStyleTable(TProjection projection, TTable table) :
			m_Projection{ std::move(projection) },
			m_StyleTable{ std::move(table) }
		{
		}

		/**
		 * \brief Invocation operator
		 * \tparam TRecord The currently used Record type
		 * \param record The actual Record object
		 * \return Returns the ConsoleTextStyle which should be applied for this Record
		 * \details If no matching established is found, defaultConsoleTextStyle will be used instead.
		 */
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

	/**
	 * \brief The factory function for creating ConsoleTextStyleTable instances
	 * \tparam TRecord The currently used Record type
	 * \tparam TProjection The projection type
	 * \tparam TTable The table type. The default type is of std::unordered_map. Users may exchange this with std::map.
	 * \param projection The projection to a member of the currently used Record type
	 * \param table The finally established container object
	 * \return Returns a newly created ConsoleTextStyleTable instance
	 * \details This is the preferable way creating a ConsoleTextStyleTable object for a Record property, because the projection and the table type
	 * becomes strong checked via concept and therefore will provide much clearer feedback in cases of error, while creating ConsoleTextStyleTable objects
	 * manually will probably result in harder to read error message. 
	 */
	template <Record TRecord,
		std::invocable<const TRecord&> TProjection,
		class TTable = std::unordered_map<
			std::remove_cvref_t<std::invoke_result_t<TProjection, const TRecord&>>,
			ConsoleTextStyle
		>>
	auto makeConsoleTextStyleTableFor(TProjection projection, TTable table)
	{
		return ConsoleTextStyleTable{ std::move(projection), std::move(table) };
	}

	/**
	 * \brief Sink class for directly logging onto std::cout
	 * \tparam TRecord Used Record type.
	 * \details This Sink class directly uses a std::cout object for printing each recorded message. Users may register \ref ConsoleTextStyle "ConsoleTextStyles" which will then colorize or stylize
	 * messages.
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
		/**
		 * \brief Constructs the default TextStylePolicy
		 * \return a invokable object
		 */
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

		/**
		 * \brief Sets the active ConsoleTextStylePolicy
		 * \tparam TStylePolicy Type of the passed ConsoleTextStylePolicy (automatically deduced)
		 * \param policy The new ConsoleTextStylePolicy object
		 */
		template <ConsoleTextStylePolicyFor<Record_t> TStylePolicy>
		void setTextStylePolicy(TStylePolicy&& policy)
		{
			std::scoped_lock lock{ m_TextStylePolicyMx };
			m_TextStylePolicy = std::forward<TStylePolicy>(policy);
		}

		/**
		 * \brief Replaces the current ConsoleTextStylePolicy with the default one
		 */
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
