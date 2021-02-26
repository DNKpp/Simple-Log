//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_CONSOLE_SINK_HPP
#define SL_LOG_CONSOLE_SINK_HPP

#pragma once

#include "Concepts.hpp"
#include "OStreamSink.hpp"

// rang.hpp includes windows.hpp internally, thus macros min and max will be defined. This macro prevents this.
#define NOMINMAX
#include "third_party/rang/rang.hpp"

#include <cassert>
#include <functional>
#include <iostream>
#include <mutex>
#include <unordered_map>

namespace sl::log
{
	/** \addtogroup Sinks
	 * @{
	 */

	/** \addtogroup ConsoleSink
	 * @{
	 * 
	 */

	/**
	 * \brief Collection of possible style and color options for text printed onto the console
	 * \details Each enum type will be casted to the third-party lib "rang" counterpart, which is responsible for all of the troublesome work.
	 * Some style options might not work on every console.
	 * 
	 * Go to https://github.com/agauniyal/rang/tree/master if you are interested about all the details.
	 */
	struct ConsoleTextStyle
	{
		enum class Color
		{
			black = 0,
			red,
			green,
			yellow,
			blue,
			magenta,
			cyan,
			gray,
			standard,

			brightBlack,
			brightRed,
			brightGreen,
			brightYellow,
			brightBlue,
			brightMagenta,
			brightCyan,
			brightGray,
		};

		enum class Style
		{
			standard = 0,
			bold,
			dim,
			italic,
			underline,
			reversed = 7,
			crossed = 9
		};

		/**
		 * \brief determines the style
		 */
		Style style = Style::standard;

		/**
		 * \brief determines the text color
		 */
		Color textColor = Color::standard;

		/**
		 * \brief determines the background color
		 */
		Color bgColor = Color::standard;
	};

	/**
	 * \brief A constant object used for resetting the style back to default
	 */
	constexpr ConsoleTextStyle defaultConsoleTextStyle;

	/** @}*/
	/** @}*/
}

namespace sl::log::detail
{
	std::ostream& applyTextColor(std::ostream& out, ConsoleTextStyle::Color color) noexcept;
	std::ostream& applyBackgroundColor(std::ostream& out, ConsoleTextStyle::Color color) noexcept;
	std::ostream& applyStyle(std::ostream& out, ConsoleTextStyle::Style style) noexcept;
}

namespace sl::log
{
	/** \addtogroup Sinks
	 * @{
	 */

	/** \addtogroup ConsoleSink
	 * @{
	 */

	/**
	 * \brief Operator << overload for ConsoleTextStyle type
	 * \param out the stream object
	 * \param style the provided style
	 * \return Returns the parameter out as reference
	 */
	inline std::ostream& operator <<(std::ostream& out, const ConsoleTextStyle& style)
	{
		detail::applyStyle(out, style.style);
		detail::applyTextColor(out, style.textColor);
		detail::applyBackgroundColor(out, style.bgColor);
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
		using Key_t = typename Table_t::key_type;

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

		/**
		 * \brief Inserts a style policy
		 * \param key The key, on which the style will be applied
		 * \param style The style which will be used, when the key is detected
		 * \details Inserts or, if key is already present, overrides the style.
		 */
		void insert(Key_t key, ConsoleTextStyle style)
		{
			m_StyleTable.insert_or_assign(std::move(key), std::move(style));
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
	 * \details This Sink class directly uses a std::cout object for printing each recorded message. Users may register \ref ConsoleTextStyle "ConsoleTextStyles" which will then colorized or
	 * printed the messages in a specific style.
	 *
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

namespace sl::log::detail
{
	inline std::ostream& applyTextColor(std::ostream& out, ConsoleTextStyle::Color color) noexcept
	{
		using Color = ConsoleTextStyle::Color;
		if (color < Color::brightBlack)
		{
			const auto begin = static_cast<unsigned>(rang::fg::black);
			out << static_cast<rang::fg>(static_cast<unsigned>(color) + begin);
		}
		else
		{
			const auto begin = static_cast<unsigned>(rang::fgB::black);
			const auto localColor = static_cast<Color>(begin +
				static_cast<unsigned>(color) -
				static_cast<unsigned>(Color::brightBlack));
			out << static_cast<rang::fgB>(localColor);
		}
		return out;
	}

	inline std::ostream& applyBackgroundColor(std::ostream& out, ConsoleTextStyle::Color color) noexcept
	{
		using Color = ConsoleTextStyle::Color;
		if (color < Color::brightBlack)
		{
			const auto begin = static_cast<unsigned>(rang::bg::black);
			out << static_cast<rang::bg>(static_cast<unsigned>(color) + begin);
		}
		else
		{
			const auto begin = static_cast<unsigned>(rang::bgB::black);
			const auto localColor = static_cast<Color>(begin +
				static_cast<unsigned>(color) -
				static_cast<unsigned>(Color::brightBlack));
			out << static_cast<rang::bgB>(localColor);
		}
		return out;
	}

	inline std::ostream& applyStyle(std::ostream& out, ConsoleTextStyle::Style style) noexcept
	{
		const auto value = static_cast<rang::style>(style);
		out << value;
		return out;
	}
}

#endif
