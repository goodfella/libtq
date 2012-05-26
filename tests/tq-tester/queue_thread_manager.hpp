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
    struct queue_thread_data
    {
	queue_thread_data(libtq::task_queue* const q, bool_flag* const f);

	libtq::task_queue* queue;
	bool_flag* stop_thread;
    };

    class queue_thread_manager
    {
	public:

	queue_thread_manager(libtq::task_queue* const queue);
	~queue_thread_manager();

	void start_threads();
	void stop_threads();

	private:

	libtq::thread m_shutdown_thread;
	libtq::thread m_cancel_tasks_thread;
	libtq::thread m_stop_thread;
	libtq::thread m_cancel_queue_thread;

	bool_flag m_stop_threads;
	queue_thread_data m_data;

	static void* shutdown_queue(void* d);
	static void* cancel_tasks(void* d);
	static void* stop_queue(void* d);
	static void* cancel_queue(void* d);
    };
}

#endif
