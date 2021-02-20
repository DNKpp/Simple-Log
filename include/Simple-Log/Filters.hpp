//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_FILTERS_HPP
#define SL_LOG_FILTERS_HPP

#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <functional>

#include "Concepts.hpp"

namespace sl::log::detail
{
	class TupleAllOf
	{
	public:
		template <class TTuple, Record TRecord>
		constexpr bool operator ()(TTuple& tuple, const TRecord& record) const
		{
			return invoke<0>(tuple, record);
		}

	private:
		template <std::size_t Index, class TTuple, Record TRecord>
		constexpr bool invoke(TTuple& tuple, const TRecord& record) const
		{
			if constexpr (Index < std::tuple_size_v<TTuple>)
			{
				if (!std::invoke(std::get<Index>(tuple), record))
				{
					return false;
				}
				return invoke<Index + 1>(tuple, record);
			}
			return true;
		}
	};

	class TupleAnyOf
	{
	public:
		template <class TTuple, Record TRecord>
		constexpr bool operator ()(TTuple& tuple, const TRecord& record) const
		{
			return invoke<0>(tuple, record);
		}

	private:
		template <std::size_t Index, class TTuple, Record TRecord>
		constexpr bool invoke(TTuple& tuple, const TRecord& record) const
		{
			if constexpr (Index < std::tuple_size_v<TTuple>)
			{
				if (std::invoke(std::get<Index>(tuple), record))
				{
					return true;
				}
				return invoke<Index + 1>(tuple, record);
			}
			return false;
		}
	};

	class TupleNoneOf :
		private TupleAnyOf
	{
		using Super = TupleAnyOf;

	public:
		template <class TTuple, Record TRecord>
		constexpr bool operator ()(TTuple& tuple, const TRecord& record) const
		{
			return !Super::operator()(tuple, record);
		}
	};
}

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
		constexpr ProjectionFilter(TProjection projection, TUnaryPredicate predicate) :
			m_Projection(std::move(projection)),
			m_Predicate(std::move(predicate))
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
		constexpr explicit FilterChain(TFilter ...filter) :
			m_Algorithm{},
			m_Filter{ std::move(filter)... }
		{
		}

		/**
		 * \brief Constructor overload
		 * \param algorithm Algorithm object
		 * \param filter Filter objects
		 */
		constexpr explicit FilterChain(TAlgorithm algorithm, TFilter ...filter) :
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
		constexpr explicit FilterAllOf(TFilter ... filter) :
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
		constexpr explicit FilterAnyOf(TFilter ... filter) :
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
		constexpr explicit FilterNoneOf(TFilter ... filter) :
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
