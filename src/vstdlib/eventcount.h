#ifndef EVENTCOUNT_H
#define EVENTCOUNT_H

#if defined( _WIN32 )
#pragma once
#endif

#include <atomic>
#include <mutex>
#include <condition_variable>

struct CEventCount {
public:
	CEventCount() :
		m_state(0),
		m_mutex(),
		m_cv()
	{};

	int PrepareWait() {
		state_t old = m_state.load(std::memory_order_relaxed);

		while (true) {
			state_t next = (old & seqMask) | ((old & waiterMask) + 1);
			
			if (m_state.compare_exchange_weak(old, next, std::memory_order_seq_cst))
				return next;
		}
	}

	void CancelWait() {
		state_t old = m_state.load(std::memory_order_relaxed);

		while (true) {
			state_t next = (old & seqMask) | ((old & waiterMask) - 1);

			if (m_state.compare_exchange_weak(old, next, std::memory_order_seq_cst))
				return;
		}
	}

	void NotifyAll() {
		state_t old = m_state.load(std::memory_order_seq_cst);

		m_mutex.lock();
		while (true) {
			if ((old & waiterMask) == 0) {
				m_mutex.unlock();
				return;
			}

			state_t next = ((old & seqMask) + 1) | (0 & waiterMask);

			if (m_state.compare_exchange_weak(old, next, std::memory_order_seq_cst))
				break;
		}
		m_mutex.unlock();
		m_cv.notify_all();
	}

	void NotifyOne() {
		state_t old = m_state.load(std::memory_order_seq_cst);

		m_mutex.lock();
		while (true) {
			if ((old & waiterMask) == 0) {
				m_mutex.unlock();
				return;
			}

			state_t next = ((old & seqMask) + 1) | ((old & waiterMask) - 1);

			if (m_state.compare_exchange_weak(old, next, std::memory_order_seq_cst))
				break;
		}
		m_mutex.unlock();
		m_cv.notify_one();
	}

	void Wait(int target) {
		std::unique_lock<std::mutex> lock(m_mutex);

		int state = m_state.load(std::memory_order_seq_cst) & ~1;
		target = target & ~1;
		if (state == target)
			m_cv.wait(lock);
	}

private:
	using state_t = uint64_t;

	// the least significant bit is the has "prepared-waiter" flag
	std::atomic<state_t> m_state;
	std::mutex m_mutex;
	std::condition_variable m_cv;

	// layout:
	// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssswwwwww
	// 6  least significant writer count bits
	// 58 most significant sequence bits
	static constexpr state_t waiterBits = 6; // 64 waiters maximum
	static constexpr state_t waiterMask = (1 << waiterBits) - 1;
	static constexpr state_t seqMask = ~waiterMask;
};

#endif