//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_FLUSH_POLICIES_HPP
#define SL_LOG_FLUSH_POLICIES_HPP

#pragma once

#include <chrono>
#include <concepts>
#include <cstddef>
#include <functional>
#include <utility>

#include "Concepts.hpp"

namespace sl::log::detail
{
	template <Record TRecord>
	class AbstractFlushPolicyWrapper
	{
	public:
		using Record_t = TRecord;

		virtual ~AbstractFlushPolicyWrapper() noexcept = default;

		virtual bool operator ()(const Record_t& record, std::size_t messageByteSize) = 0;
		virtual void flushed() = 0;

	protected:
		AbstractFlushPolicyWrapper() noexcept = default;
	};

	template <Record TRecord, FlushPolicyFor<TRecord> TFlushPolicy>
	class FlushPolicyWrapper final :
		public AbstractFlushPolicyWrapper<TRecord>
	{
		using Super = AbstractFlushPolicyWrapper<TRecord>;
	public:
		using Record_t = std::remove_cvref_t<TRecord>;
		using FlushPolicy_t = std::remove_reference_t<TFlushPolicy>;

		explicit FlushPolicyWrapper() noexcept(std::is_nothrow_constructible_v<FlushPolicy_t>) :
			m_FlushPolicy{}
		{
		}

		explicit FlushPolicyWrapper(TFlushPolicy flushPolicy) noexcept(std::is_nothrow_move_constructible_v<FlushPolicy_t>) :
			m_FlushPolicy{ std::move(flushPolicy) }
		{
		}

		bool operator ()(const Record_t& record, std::size_t messageByteSize) override
		{
			return std::invoke(m_FlushPolicy, record, messageByteSize);
		}

		void flushed() override
		{
			m_FlushPolicy.flushed();
		}

	private:
		FlushPolicy_t m_FlushPolicy;
	};

	template <auto Constant>
	struct ConstantInvokable
	{
		template <class... TArgs>
		constexpr auto operator()(TArgs&&...) const noexcept
		{
			return Constant;
		}
	};

	struct PredProjInvocation
	{
		template <class TPredicate, class TProjection, class TProjected, class... TArgs>
		constexpr auto operator ()(TPredicate& pred, TProjection& proj, const TProjected& projected, TArgs&&... args) const
		{
			return std::invoke(pred, std::invoke(proj, projected), std::forward<TArgs>(args)...);
		}
	};

	struct PredProjInvocationIgnoreArgs
	{
		template <class TPredicate, class TProjection, class TProjected, class... TArgs>
		constexpr auto operator ()(TPredicate& pred, TProjection& proj, const TProjected& projected, TArgs&&...) const
		{
			return std::invoke(pred, std::invoke(proj, projected));
		}
	};
}

namespace sl::log
{
	/** \addtogroup Flush-Policy
	 * @{
	 *
	 * \details Flush-Policies determine if the Sink, they are attached to, shall flush its stream. They will be invoked after each handled Record and shall return
	 * true if a flush should happen. After flushing the Sink will call the flushed() function, which Flush-Policies can then use to reset their internal state if necessary.
	 *
	 * Users can use the provided Flush-Policy classes or create their own. Custom policies do not have to inherit from any base-class but must follow the interface defined by the FlushPolicyFor concept.
	 */

	/**
	 * \brief A customizable FlushPolicy class
	 * \tparam TPredicate The predicate must be invokable with the result of TProjection and must boolean-comparable results.
	 * \tparam TProjection The projection must be invokable with the used Record type and must return something non-void.
	 * \tparam TInvocationRule This type is used for internal determination how to chain the invocations.
	 *
	 * \details This is a customizable class when a Flush-Policy does not carrie any internal state which has to be reset after flushing happened. In other words,
	 * users can pass any type of predicate, which fulfills the requirements, but will never receive the flushed() signal. If you need to act on that signal, users can simply
	 * write their own FlushPolicy class. There is no base class FlushPolicies must inherit from.
	 * Additionally it is possible to use a projection, which will be invoked before the actual predicate. This projection must accept an object of the used Record type as single parameter.
	 */
	template <class TPredicate, class TProjection = std::identity, class TInvocationRule = detail::PredProjInvocation>
	class FlushPolicy
	{
	public:
		using Predicate_t = std::remove_cvref_t<TPredicate>;
		using Projection_t = std::remove_cvref_t<TProjection>;
		using InvocationRule_t = std::remove_cvref_t<TInvocationRule>;

		/**
		 * \brief Constructor
		 * \param predicate Used predicate. May be default constructed via default argument.
		 * \param projection Used projection. May be default constructed via default argument.
		 * \param invocation Used invocation rule. May be default constructed via default argument.
		 */
		explicit FlushPolicy(
			TPredicate predicate = Predicate_t{},
			TProjection projection = Projection_t{},
			TInvocationRule invocation = InvocationRule_t{}
		) :
			m_Predicate{ std::move(predicate) },
			m_Projection{ std::move(projection) },
			m_Invocation{ std::move(invocation) }
		{
		}

		/**
		 * \brief Constructor overload for in_place predicate initializing
		 * \tparam TArgs Types of the predicate constructor arguments 
		 * \param _ Dummy value used for detecting the overload
		 * \param args Constructor arguments for the predicate
		 * \details Constructs the predicate in-place
		 */
		template <class... TArgs>
		requires std::constructible_from<Predicate_t, TArgs...>
		explicit FlushPolicy(std::in_place_t _, TArgs&&... args) :
			m_Predicate{ std::forward<TArgs>(args)... }
		{
		}

		/**
		 * \brief Invoke operator
		 * \tparam TRecord Used Record type
		 * \param record The current handled Record object
		 * \param messageByteSize The size of the current handled message in bytes
		 * \return Returns the result of the predicate invocation
		 * \details Just invokes the predicate with the result of the invoked projection and messageByteSize
		 */
		template <Record TRecord>
		bool operator ()(const TRecord& record, std::size_t messageByteSize)
		{
			return m_Invocation(m_Predicate, m_Projection, record, messageByteSize);
		}

		/**
		 * \brief NoOp
		 */
		constexpr void flushed() const noexcept
		{
		}

	private:
		Predicate_t m_Predicate;
		Projection_t m_Projection;
		//[[no_unique_address]]
		InvocationRule_t m_Invocation;
	};

	/**
	 * \brief A Flush-Policy which returns always true
	 */
	using AlwaysFlushPolicy = FlushPolicy<detail::ConstantInvokable<true>>;

	/**
	 * \brief Factory function for creating Flush-Policies based on Record::message member
	 * \tparam TRecord Concrete Record type on which to apply the projection
	 * \tparam TUnaryPredicate Invokable type, which has to accept objects the actual Record::Message_t type
	 * \param predicate Predicate object
	 * \return Flush-Policy object
	 * \details This is the preferable way creating a Flush-Policies based on the Record::message member, because the predicate becomes strong checked via
	 * concept and therefore will provide much clearer feedback in cases of error, while creating such Flush-Policies objects manually will
	 * potentially result in harder to read error messages. 
	 */
	template <Record TRecord, std::predicate<const typename TRecord::SeverityLevel_t&> TUnaryPredicate>
	constexpr auto makeMessageFlushPolicyFor(TUnaryPredicate&& predicate)
	{
		return FlushPolicy{
			std::forward<TUnaryPredicate>(predicate),
			&TRecord::message,
			detail::PredProjInvocationIgnoreArgs{}
		};
	}

	/**
	 * \brief Factory function for creating Flush-Policies based on Record::severity member
	 * \tparam TRecord Concrete Record type on which to apply the projection
	 * \tparam TUnaryPredicate Invokable type, which has to accept objects the actual Record::SeverityLevel_t type
	 * \param predicate Predicate object
	 * \return Flush-Policy object
	 * \details This is the preferable way creating a Flush-Policies based on the Record::severity member, because the predicate becomes strong checked via
	 * concept and therefore will provide much clearer feedback in cases of error, while creating such Flush-Policies objects manually will
	 * potentially result in harder to read error messages. 
	 */
	template <Record TRecord, std::predicate<const typename TRecord::SeverityLevel_t&> TUnaryPredicate>
	constexpr auto makeSeverityFlushPolicyFor(TUnaryPredicate&& predicate)
	{
		return FlushPolicy{
			std::forward<TUnaryPredicate>(predicate),
			&TRecord::severity,
			detail::PredProjInvocationIgnoreArgs{}
		};
	}

	/**
	 * \brief Factory function for creating Flush-Policies based on Record::channel member
	 * \tparam TRecord Concrete Record type on which to apply the projection
	 * \tparam TUnaryPredicate Invokable type, which has to accept objects the actual Record::Channel_t type
	 * \param predicate Predicate object
	 * \return Flush-Policy object
	 * \details This is the preferable way creating a Flush-Policies based on the Record::channel member, because the predicate becomes strong checked via
	 * concept and therefore will provide much clearer feedback in cases of error, while creating such Flush-Policies objects manually will
	 * potentially result in harder to read error messages. 
	 */
	template <Record TRecord, std::predicate<const typename TRecord::Channel_t&> TUnaryPredicate>
	constexpr auto makeChannelFlushPolicyFor(TUnaryPredicate&& predicate)
	{
		return FlushPolicy{
			std::forward<TUnaryPredicate>(predicate),
			&TRecord::channel,
			detail::PredProjInvocationIgnoreArgs{}
		};
	}

	/**
	 * \brief Factory function for creating Flush-Policies based on Record::timePoint member
	 * \tparam TRecord Concrete Record type on which to apply the projection
	 * \tparam TUnaryPredicate Invokable type, which has to accept objects the actual Record::TimePoint_t type
	 * \param predicate Predicate object
	 * \return Flush-Policy object
	 * \details This is the preferable way creating a Flush-Policies based on the Record::timePoint member, because the predicate becomes strong checked via
	 * concept and therefore will provide much clearer feedback in cases of error, while creating such Flush-Policies objects manually will
	 * potentially result in harder to read error messages. 
	 */
	template <Record TRecord, std::predicate<const typename TRecord::TimePoint_t&> TUnaryPredicate>
	constexpr auto makeTimePointFlushPolicyFor(TUnaryPredicate&& predicate)
	{
		return FlushPolicy{
			std::forward<TUnaryPredicate>(predicate),
			&TRecord::timePoint,
			detail::PredProjInvocationIgnoreArgs{}
		};
	}

	/**
	 * \brief A Flush-Policy which acts on a durations
	 * \details Instances of this class will return true if their given duration has been exceeded. This class uses durations based on
	 * milliseconds, thus any provided duration will be casted to that.
	 * 
	 */
	class TimedFlushPolicy
	{
	public:
		using Duration_t = std::chrono::milliseconds;
		using Clock_t = std::chrono::steady_clock;
		using TimePoint_t = Clock_t::time_point;

		/**
		 * \brief Constructs the object with the specified duration threshold
		 * \tparam TRep An arithmetic type representing the number of ticks
		 * \tparam TPeriod Period type
		 * \param duration The duration threshold.
		 * \details Any provided duration will be casted to milliseconds, thus finer thresholds will be lost.
		 */
		template <class TRep, class TPeriod>
		explicit TimedFlushPolicy(std::chrono::duration<TRep, TPeriod> duration) :
			m_Duration{ std::chrono::duration_cast<Duration_t>(duration) }
		{
		}

		/**
		 * \brief Invoke operator
		 * \tparam TRecord Used Record type
		 * \param record The current handled Record object
		 * \param messageByteSize The size of the current handled message in bytes
		 * \return Returns true if the duration has been exceeded.
		 */
		template <Record TRecord>
		bool operator ()(const TRecord& record, std::size_t messageByteSize) noexcept
		{
			const auto now = Clock_t::now();
			return m_Duration <= now - m_StartPoint;
		}

		/**
		 * \brief Restarts the internal duration clock
		 */
		void flushed() noexcept
		{
			m_StartPoint = Clock_t::now();
		}

	private:
		TimePoint_t m_StartPoint = Clock_t::now();
		Duration_t m_Duration;
	};

	/**
	 * \brief A Flush-Policy which acts on accumulated byte count
	 * \details Instances of this class will return true if their given byte threshold has been exceeded.
	 */
	class ByteCountFlushPolicy
	{
	public:
		/**
		 * \brief Constructor
		 * \param byteThreshold Threshold size in bytes 
		 */
		explicit ByteCountFlushPolicy(std::size_t byteThreshold) :
			m_ByteThreshold{ byteThreshold }
		{
		}

		/**
		 * \brief Invoke operator
		 * \tparam TRecord Used Record type
		 * \param record The current handled Record object
		 * \param messageByteSize The size of the current handled message in bytes
		 * \return Returns true if the byte threshold has been exceeded.
		 */
		template <Record TRecord>
		bool operator ()(const TRecord& record, std::size_t messageByteSize) noexcept
		{
			m_ByteCount += messageByteSize;
			return m_ByteThreshold <= m_ByteCount;
		}

		/**
		 * \brief Resets the internal byte counter
		 */
		void flushed() noexcept
		{
			m_ByteCount = 0;
		}

	private:
		std::size_t m_ByteThreshold;
		std::size_t m_ByteCount = 0;
	};

	/** @}*/
}

#endif
