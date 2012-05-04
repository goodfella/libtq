#ifndef TASK_QUEUE_HPP
#define TASK_QUEUE_HPP

#include <pthread.h>
#include <list>

#include "task_desc.hpp"

namespace libtq
{
    class itask;

    class task_queue
    {
	public:

	task_queue();
	~task_queue();

	bool start_queue();
	void stop_queue();

	void queue_task(itask * const task);
	int wait_for_task(itask * const task);
	int cancel_task(itask * const task);

	private:

	bool m_started;
	bool m_shutdown;
	std::list<task_desc> m_tasks;
	pthread_mutex_t m_lock;
	pthread_cond_t m_cond;
	pthread_t m_thread;

	// assumes m_lock is held prior to being called
	void priv_queue_task(itask * const task);

	static void* task_runner(void* task_queue);
	static void handle_cancelation(void* task_queue);
    };
}

#endif
