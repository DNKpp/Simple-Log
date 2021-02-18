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
		using Record_t = TRecord;
		using FlushPolicy_t = TFlushPolicy;

		explicit FlushPolicyWrapper() noexcept(std::is_nothrow_constructible_v<TFlushPolicy>) :
			m_FlushPolicy{}
		{
		}

		explicit FlushPolicyWrapper(TFlushPolicy flushPolicy) noexcept(std::is_nothrow_move_constructible_v<TFlushPolicy>) :
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
		TFlushPolicy m_FlushPolicy;
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
}

namespace sl::log
{
	/** \addtogroup Flushing
	 * @{
	 */

	template <class TPredicate>
	class FlushPolicy
	{
	public:
		using Predicate_t = TPredicate;

		explicit FlushPolicy(Predicate_t predicate = Predicate_t{}) :
			m_Predicate{ std::move(predicate) }
		{
		}

		template <class... TArgs>
		requires std::constructible_from<Predicate_t, TArgs...>
		explicit FlushPolicy(std::in_place_t _, TArgs&&... args) :
			m_Predicate{ std::forward<TArgs>(args)... }
		{
		}

		template <Record TRecord>
		bool operator ()(const TRecord& record, std::size_t messageByteSize)
		{
			return std::invoke(m_Predicate, record, messageByteSize);
		}

		constexpr void flushed() const noexcept
		{
		}

	private:
		Predicate_t m_Predicate;
	};

	using AlwaysFlushPolicy = FlushPolicy<detail::ConstantInvokable<true>>;

	class TimedFlushPolicy
	{
	public:
		using Duration_t = std::chrono::milliseconds;
		using Clock_t = std::chrono::steady_clock;
		using TimePoint_t = Clock_t::time_point;

		template <class TRep, class TPeriod>
		explicit TimedFlushPolicy(std::chrono::duration<TRep, TPeriod> duration) :
			m_Duration{ std::chrono::duration_cast<Duration_t>(duration) }
		{
		}

		template <Record TRecord>
		bool operator ()(const TRecord& record, std::size_t messageByteSize) noexcept
		{
			const auto now = Clock_t::now();
			return m_Duration <= now - m_StartPoint;
		}

		void flushed() noexcept
		{
			m_StartPoint = Clock_t::now();
		}

	private:
		TimePoint_t m_StartPoint = Clock_t::now();
		Duration_t m_Duration;
	};

	class ByteCountFlushPolicy
	{
	public:
		explicit ByteCountFlushPolicy(std::size_t byteThreshold) :
			m_ByteThreshold{ byteThreshold }
		{
		}

		template <Record TRecord>
		bool operator ()(const TRecord& record, std::size_t messageByteSize) noexcept
		{
			m_ByteCount += messageByteSize;
			return m_ByteThreshold <= m_ByteCount;
		}

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
