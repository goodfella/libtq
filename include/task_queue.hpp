#ifndef TASK_QUEUE_HPP
#define TASK_QUEUE_HPP

#include <pthread.h>
#include <list>

#include "task_desc.hpp"

namespace libtq
{
    class itask;
    class task_handle;
    class task_runner_cleanup;

    class task_queue
    {
	friend class task_handle;
	friend class task_runner_cleanup;

	public:

	task_queue();
	~task_queue();

	int start_queue();
	int stop_queue();

	void queue_task(itask * const task);
	int wait_for_task(itask * const task);
	int cancel_task(itask * const task, bool& cancel_status);

	private:

	bool m_started;
	std::list<task_desc> m_tasks;
	pthread_mutex_t m_lock;
	pthread_mutex_t m_shutdown_lock;
	pthread_cond_t m_cond;
	pthread_cond_t m_shutdown_cond;
	pthread_t m_thread;

	/** Queues and waits for a task atomically
	 *
	 *  The task is queued and then waited on.  If the task was
	 *  already queued, then it's not waited on.  So callers
	 *  should make sure the task is not already queued prior to
	 *  calling this function.
	 *
	 *  @return zero if the task was allready queued, or if the
	 *  task was queued, and the wait succeeded.  Non-zero if an
	 *  error occured while trying to wait on the task.
	 */
	int queue_task_wait(itask * const task);

	/** Queues a task
	 *
	 *  @return true if the task was queued, false otherwise
	 *
	 *  @notes Assumes m_lock is held prior to being called
	 */
	bool priv_queue_task(itask * const task);

	// assumes m_lock is held prior to being called
	int priv_wait_for_task(itask* const task);

	// assumes m_lock is held prior to being called
	int priv_wait_for_task(task_desc& desc);

	static void* task_runner(void* task_queue);
	static void handle_cancelation(void* task_queue);
    };
}

#endif
