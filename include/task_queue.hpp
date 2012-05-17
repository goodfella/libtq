#ifndef TASK_QUEUE_HPP
#define TASK_QUEUE_HPP

#include <pthread.h>

#include "itask_queue.hpp"

namespace libtq
{
    class itask;

    class task_queue
    {
	public:

	task_queue();
	~task_queue();

	/** Starts the task queue's task runner thread
	 *
	 *  @return zero if the task runner thread was successfully
	 *  started, non-zero otherwise.
	 */
	int start_queue();

	/** Stops the task queue's task runner thread
	 *
	 *  The calling thread will block until the task runner has
	 *  been shut down successfully.
	 */
	void stop_queue();

	/** Queues a task
	 *
	 *  A task can only be queued once.  So if the task was
	 *  already queued and is not yet complete, this function will
	 *  not queue the task a second time.
	 */
	void queue_task(itask * const task);

	/** Waits for a task to complete
	 *
	 *  The calling thread will block until the task execution
	 *  completes.  If the task is not already queued, this
	 *  function exits immediately.
	 *
	 *  @return true if the task was waited on, false otherwise
	 */
	bool wait_for_task(itask * const task);

	/** Cancels the execution of a task
	 *
	 *  Cancels a previously queued task, and notifies all the
	 *  threads waiting on the task that the task is complete.
	 *
	 *  @return true if the task was canceled, false if the task
	 *  was not scheduled at the time this method was called.
	 */
	bool cancel_task(itask * const task);

	private:

	/// Predicate used to determine if the task runner thread is started
	bool m_started;

	/// Protects the m_started predicate.
	pthread_mutex_t m_shutdown_lock;

	/// Task runner thread handle
	pthread_t m_thread;

	/// Underlying task queue
	itask_queue m_queue;

	static void* task_runner(void* task_queue);
    };

    inline void task_queue::queue_task(itask * const taskp)
    {
	m_queue.queue_task(taskp);
    }

    inline bool task_queue::wait_for_task(itask * const taskp)
    {
	return m_queue.wait_for_task(taskp);
    }

    inline bool task_queue::cancel_task(itask * const taskp)
    {
	return m_queue.cancel_task(taskp);
    }
}

#endif
