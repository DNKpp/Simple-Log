//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_FILTERS_HPP
#define SL_LOG_FILTERS_HPP

#pragma once

#include <algorithm>
#include <concepts>
#include <tuple>
#include <functional>
#include <type_traits>

#include "Concepts.hpp"
#include "TupleAlgorithms.hpp"

namespace sl::log
{
	/** \addtogroup Filter
	 * @{
	 * \details Filters operate on Sink level and determine if a Record will be handled by that Sink object or skipped. Users are able apply provided filter like the ProjectionFilter types
	 * or use custom types. Those custom types can be from any type which is invokable by the used Record type and returning a bool (or at least a bool-comparable-type) as result.
	 *
	 * There are already some convenience factory function present (e.g. makeMessageProjectionFor), which may help setting up filters quickly.
	 */

	/**
	 * \brief Combines a projection on Record type with a predicate into an invokable object
	 * \tparam TProjection A Projection type, which has to accept an object of the used Record type and must return something
	 * \tparam TUnaryPredicate A predicate type, which accepts objects returned by TProjection
	 * \details Instances of this class are invokable with any types, which follow the Record concept. Due to possible custom implementations
	 * or adjustments ProjectionFilter can't rely on the libraries BaseRecord type and therefore is not able to pre-check the provided Projection and
	 * Predicate via concept.
	 */
	template <class TProjection, class TUnaryPredicate>
	class ProjectionFilter
	{
	public:
		using Projection_t = TProjection;
		using UnaryPredicate_t = TUnaryPredicate;

		/**
		 * \brief Constructor
		 * \param projection Invokable object
		 * \param predicate Predicate object
		 */
		constexpr ProjectionFilter(
			TProjection projection,
			TUnaryPredicate predicate
		)
		noexcept(std::is_nothrow_move_constructible_v<TProjection> && std::is_nothrow_move_constructible_v<TUnaryPredicate>) :
			m_Projection{ std::move(projection) },
			m_Predicate{ std::move(predicate) }
		{
		}

		/**
		 * \brief Call-operator
		 * \tparam TRecord Concrete type of the used Record object
		 * \param rec Record object
		 * \return Returns true, if the Record should be handled. False if skipped.
		 */
		template <Record TRecord>
		constexpr bool operator ()(const TRecord& rec)
		{
			return std::invoke(m_Predicate, std::invoke(m_Projection, rec));
		}

	private:
		Projection_t m_Projection;
		UnaryPredicate_t m_Predicate;
	};

	/**
	 * \brief Chains multiple filter together
	 * \tparam TAlgorithm The used algorithm, which will determine how each invoke result will be treated.
	 * \tparam TFilter Type of provided filter objects
	 * \details This class simply calls each provided filter successively. The provided algorithm will determine, how each invocation result will be treated.
	 */
	template <class TAlgorithm, class... TFilter>
	class FilterChain
	{
	public:
		/**
		 * \brief Constructor
		 * \param filter Filter objects
		 */
		constexpr explicit FilterChain(
			TFilter ...filter
		)
		noexcept(std::is_nothrow_constructible_v<TAlgorithm> && (std::is_nothrow_move_constructible_v<TFilter> && ...)) :
			m_Algorithm{},
			m_Filter{ std::move(filter)... }
		{
		}

		/**
		 * \brief Constructor overload
		 * \param algorithm Algorithm object
		 * \param filter Filter objects
		 */
		constexpr explicit FilterChain(
			TAlgorithm algorithm,
			TFilter ...filter
		) noexcept((std::is_nothrow_move_constructible_v<TFilter> && ...)) :
			m_Algorithm(std::move(algorithm)),
			m_Filter{ std::forward<TFilter>(filter)... }
		{
		}

		/**
		 * \brief Call-operator
		 * \tparam TRecord Concrete type of the used Record object
		 * \param rec Record object
		 * \return Returns true, if the Record should be handled. False if skipped.
		 */
		template <Record TRecord>
		constexpr bool operator()(const TRecord& rec)
		{
			return std::invoke(m_Algorithm, m_Filter, rec);
		}

		/**
		 * \brief Returns whether the are no sub-filters attached.
		 * \return true if there aren't have any sub-filters attached.
		 */
		[[nodiscard]]
		constexpr bool empty() const noexcept
		{
			return std::tuple_size_v<decltype(m_Filter)> == 0;
		}

		/**
		 * \brief Obtains the amount of attached sub-filters
		 * \return The amount of attached sub-filters
		 */
		[[nodiscard]]
		constexpr std::size_t size() const noexcept
		{
			return std::tuple_size_v<decltype(m_Filter)>;
		}

	private:
		TAlgorithm m_Algorithm;
		std::tuple<TFilter...> m_Filter;
	};

	//ToDo: Clang currently doesn't support alias CTAD: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1814r0.html
	//template <std::predicate<const Record&>... TFilter>
	//using FilterDisjunction = FilterChain<FilterAllOf, TFilter...>;
	// 
	//template <std::predicate<const Record&>... TFilter>
	//using FilterConjunction = FilterChain<FilterAnyOf, TFilter...>;

	/**
	 * \brief Convenience type for chaining multiple filter with AND
	 * \tparam TFilter Type of provided filter objects
	 */
	template <class... TFilter>
	class FilterAllOf :
		public FilterChain<detail::TupleAllOf, TFilter...>
	{
		using Algorithm = detail::TupleAllOf;

	public:
		/**
		 * \brief Constructor
		 * \param filter Filter objects
		 */
		constexpr explicit FilterAllOf(
			TFilter ... filter
		)
		noexcept((std::is_nothrow_move_constructible_v<TFilter> && ...)) :
			FilterChain<Algorithm, TFilter...>{ std::move(filter)... }
		{
		}
	};

	/**
	 * \brief Convenience type for chaining multiple filter with OR
	 * \tparam TFilter Type of provided filter objects
	 */
	template <class... TFilter>
	class FilterAnyOf :
		public FilterChain<detail::TupleAnyOf, TFilter...>
	{
		using Algorithm = detail::TupleAnyOf;

	public:
		/**
		 * \brief Constructor
		 * \param filter Filter objects
		 */
		constexpr explicit FilterAnyOf(
			TFilter ... filter
		)
		noexcept((std::is_nothrow_move_constructible_v<TFilter> && ...)) :
			FilterChain<Algorithm, TFilter...>{ std::move(filter)... }
		{
		}
	};

	/**
	 * \brief Convenience type for chaining multiple filter with NOR
	 * \tparam TFilter Type of provided filter objects
	 */
	template <class... TFilter>
	class FilterNoneOf :
		public FilterChain<detail::TupleNoneOf, TFilter...>
	{
		using Algorithm = detail::TupleNoneOf;

	public:
		/**
		 * \brief Constructor
		 * \param filter Filter objects
		 */
		constexpr explicit FilterNoneOf(
			TFilter ... filter
		)
		noexcept((std::is_nothrow_move_constructible_v<TFilter> && ...)) :
			FilterChain<Algorithm, TFilter...>{ std::move(filter)... }
		{
		}
	};

	/**
	 * \brief Factory function for creating ProjectionFilter of Record::message member
	 * \tparam TRecord Concrete Record type on which to apply the projection
	 * \tparam TUnaryPredicate Invokable type, which has to accept objects the actual Record::Message_t type
	 * \param predicate Predicate object
	 * \return ProjectionFilter object
	 * \details This is the preferable way creating a filter for the Record::message member, because the predicate becomes strong checked via
	 * concept and therefore will provide much clearer feedback in cases of error, while creating ProjectionFilter objects manually will
	 * potentially result in harder to read error message. 
	 */
	template <Record TRecord, std::predicate<const typename TRecord::Message_t&> TUnaryPredicate>
	constexpr auto makeMessageFilterFor(TUnaryPredicate&& predicate)
	{
		return ProjectionFilter{ &TRecord::message, std::forward<TUnaryPredicate>(predicate) };
	}

	/**
	 * \brief Factory function for creating ProjectionFilter of Record::severity member
	 * \tparam TRecord Concrete Record type on which to apply the projection
	 * \tparam TUnaryPredicate Invokable type, which has to accept objects the actual Record::SeverityLevel_t type
	 * \param predicate Predicate object
	 * \return ProjectionFilter object
	 * \details This is the preferable way creating a filter for the Record::severity member, because the predicate becomes strong checked via
	 * concept and therefore will provide much clearer feedback in cases of error, while creating ProjectionFilter objects manually will
	 * potentially result in harder to read error message. 
	 */
	template <Record TRecord, std::predicate<const typename TRecord::SeverityLevel_t&> TUnaryPredicate>
	constexpr auto makeSeverityFilterFor(TUnaryPredicate&& predicate)
	{
		return ProjectionFilter{ &TRecord::severity, std::forward<TUnaryPredicate>(predicate) };
	}

	/**
	 * \brief Factory function for creating ProjectionFilter of Record::channel member
	 * \tparam TRecord Concrete Record type on which to apply the projection
	 * \tparam TUnaryPredicate Invokable type, which has to accept objects the actual Record::Channel_t type
	 * \param predicate Predicate object
	 * \return ProjectionFilter object
	 * \details This is the preferable way creating a filter for the Record::channel member, because the predicate becomes strong checked via
	 * concept and therefore will provide much clearer feedback in cases of error, while creating ProjectionFilter objects manually will
	 * potentially result in harder to read error message. 
	 */
	template <Record TRecord, std::predicate<const typename TRecord::Channel_t&> TUnaryPredicate>
	constexpr auto makeChannelFilterFor(TUnaryPredicate&& predicate)
	{
		return ProjectionFilter{ &TRecord::channel, std::forward<TUnaryPredicate>(predicate) };
	}

	/**
	 * \brief Factory function for creating ProjectionFilter of Record::timePoint member
	 * \tparam TRecord Concrete Record type on which to apply the projection
	 * \tparam TUnaryPredicate Invokable type, which has to accept objects the actual Record::TimePoint_t type
	 * \param predicate Predicate object
	 * \return ProjectionFilter object
	 * \details This is the preferable way creating a filter for the Record::timePoint member, because the predicate becomes strong checked via
	 * concept and therefore will provide much clearer feedback in cases of error, while creating ProjectionFilter objects manually will
	 * potentially result in harder to read error message. 
	 */
	template <Record TRecord, std::predicate<const typename TRecord::TimePoint_t&> TUnaryPredicate>
	constexpr auto makeTimePointFilterFor(TUnaryPredicate&& predicate)
	{
		return ProjectionFilter{ &TRecord::timePoint, std::forward<TUnaryPredicate>(predicate) };
	}

	/** @}*/
}

#endif
