#ifndef QUEUE_THREAD_MANAGER_HPP
#define QUEUE_THREAD_MANAGER_HPP

#include <thread>

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

	void start_threads();
	void stop_threads();

	private:

	std::thread m_shutdown_thread;
	std::thread m_stop_thread;
	std::thread m_flush_thread;

	bool_flag m_stop_threads;
	queue_thread_data m_data;

	static void shutdown_queue(queue_thread_data* d);
	static void stop_queue(queue_thread_data* d);
	static void flush_queue(queue_thread_data* d);
    };
}

#endif
