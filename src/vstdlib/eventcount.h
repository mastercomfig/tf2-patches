#ifndef EVENTCOUNT_H
#define EVENTCOUNT_H

#if defined( _WIN32 )
#pragma once
#endif

#include <atomic>
#include <mutex>
#include <condition_variable>

struct CEventCount {
// largely inspired by https://cbloomrants.blogspot.com/2011/07/07-08-11-event-count-and-condition.html
public:
	CEventCount() :
		m_count(0),
		m_mutex(),
		m_cv()
	{};

	int PrepareWait() {
		return m_count.fetch_or(1, std::memory_order_acquire);
	}

	void NotifyAll() {
		int x = m_count.fetch_add(0, std::memory_order_seq_cst);
		if ((x & 1) == 0)
			return;

		m_mutex.lock();
		while (!m_count.compare_exchange_weak(x, (x + 2) & ~1, std::memory_order_seq_cst));
		m_mutex.unlock();
		m_cv.notify_all();
	}

	void Wait(int target) {
		std::unique_lock<std::mutex> lock(m_mutex);

		int state = m_count.load(std::memory_order_seq_cst) & ~1;
		target = target & ~1;
		if (state == target)
			m_cv.wait(lock);
	}

private:
	// the least significant bit is the has "prepared-waiter" flag
	std::atomic<int> m_count;
	std::mutex m_mutex;
	std::condition_variable m_cv;
};

#endif