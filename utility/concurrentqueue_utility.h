#ifndef __CONCURRENTQUEUE_UTILITY_H__
#define __CONCURRENTQUEUE_UTILITY_H__

#include "./concurrentqueue/concurrentqueue.h"
#include "./concurrentqueue/blockingconcurrentqueue.h"

namespace util
{

template<typename T, typename Traits = moodycamel::ConcurrentQueueDefaultTraits>
using concurrent_queue = ::moodycamel::ConcurrentQueue<T, Traits>;

template<typename T, typename Traits = moodycamel::ConcurrentQueueDefaultTraits>
using blocking_concurrent_queue = ::moodycamel::BlockingConcurrentQueue<T, Traits>;

} /* namespace util */
#endif /* __CONCURRENTQUEUE_UTILITY_H__ */