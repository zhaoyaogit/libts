#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <memory>
#include <thread>
#include <assert.h>
#include <boost/lockfree/spsc_queue.hpp>
#include "ts3/timestamp.hpp"

using namespace ts3;

template <typename T, size_t rgSize=1024> 
class RingBuffer {
public:
	RingBuffer<T,rgSize>(const int timeout_Ms=1000, const bool bYield=true):
			nWaits_(0),
			yield_(bYield),
			timeout_(timeout_Ms*1e6) {}
	RingBuffer<T,rgSize>(const RingBuffer&& ringB):
			nWaits_(ringB.nWais),
			yield_(ringB.yield_),
			timeout_(ringB.timeout_)
	{
		queue_ = std::move(ringB.queue_);
	}
	bool push(const T &dat) {
		timespec tpS;
		clock_gettime(CLOCK_REALTIME, &tpS);
		while (!queue_.push(dat)) {
			if (yield_) std::this_thread::yield();
			if (timeout_ > 0) {
				timespec tpE;
				clock_gettime(CLOCK_REALTIME, &tpE);
				if (tpE - tpS > timeout_) return false;
			}
		}
		return true;
	}
	bool pop(T &dat) {
		return queue_.pop(dat);
	}
	bool isEmpty() { return queue_.empty(); }
	bool is_lockfree() { return queue_.is_lock_free(); }

	size_t	waitCount() { return nWaits_; }

private:
	boost::lockfree::spsc_queue<T, boost::lockfree::capacity<rgSize> > queue_;
	size_t	nWaits_;
	bool	yield_;
	int64_t	timeout_;
};