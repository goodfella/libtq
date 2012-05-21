#ifndef QUEUE_THREAD_MANAGER_HPP
#define QUEUE_THREAD_MANAGER_HPP

#include <pthread.h>

#include "thread.hpp"
#include "bool_flag.hpp"

namespace libtq
{
    class task_queue;
}

namespace tq_tester
{
    class queue_thread_manager
    {
	public:

	queue_thread_manager(libtq::task_queue* const queue);
	~queue_thread_manager();

	void start_thread();
	void stop_thread();

	private:

	libtq::task_queue* m_queue;
	libtq::thread m_stop_thread;

	static bool_flag m_stop_threads;
	static void* queue_thread(void*);
    };
}

#endif
