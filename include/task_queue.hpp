#ifndef TASK_QUEUE_HPP
#define TASK_QUEUE_HPP

#include <pthread.h>
#include <list>

#include "task.hpp"

namespace libtq
{
    class itask;
    class task_runner_cleanup;

    class task_queue
    {
	friend class task_runner_cleanup;

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
	 *  threads waiting on the task that the task is complete.  If
	 *  the task was not already queued, this function exits
	 *  immediately.  If the task is already queued, and the task
	 *  runner is not preparing to run the task, then the task is
	 *  canceled, and the threads waiting on the task are notified
	 *  that the task is complete.  If the task is already queued,
	 *  and the task runner is preparing to run the task, then
	 *  this function works just like wait_for_task i.e. the
	 *  calling thread blocks until the task execution completes.
	 *
	 *  @return true if the task was canceled, false if the task
	 *  was not canceled either because the task was not queued,
	 *  or because the task was pending.
	 */
	bool cancel_task(itask * const task);

	private:

	// Predicate used to determine if the task runner thread is
	bool m_started;

	// Predicate used to insure only one thread goes through the
	// entire stop_queue routine
	bool m_shutdown_pending;

	// List of tasks to run
	std::list<task> m_tasks;

	// Protects the m_tasks list.  If m_shutdown_lock and m_lock
	// both need to be taken, then the m_shutdown_lock must be
	// taken first.
	pthread_mutex_t m_lock;

	// Protects the m_started predicate.  If m_shutdown_lock and
	// m_lock both need to be taken, then the m_shutdown_lock must
	// be taken first.
	pthread_mutex_t m_shutdown_lock;

	// Used to signal the task runner waiting on an empty task
	// list
	pthread_cond_t m_cond;

	// Used to signal threads waiting on the termination of the
	// task runner thread.
	pthread_cond_t m_shutdown_cond;

	// Task runner thread handle
	pthread_t m_thread;

	/** Queues and waits for a task atomically
	 *
	 *  The task is queued and then waited on.  If the task was
	 *  already queued prior to calling this function, then it's
	 *  not waited on.  So callers should make sure the task is
	 *  not already queued prior to calling this function.
	 *
	 *  @return true if the task was queued, false if the task was
	 *  already queued prior to calling this function.
	 */
	bool queue_task_wait(itask * const task);

	/** Queues a task
	 *
	 *  @return true if the task was queued, false otherwise
	 *
	 *  @note Assumes m_lock is held prior to being called
	 */
	bool priv_queue_task(itask * const task);

	// Searches for the task and waits on it if it's queued.
	// Assumes m_lock is held prior to being called
	bool priv_wait_for_task(itask* const task);

	// Waits on the task passed in, assumes m_lock is held
	void priv_wait_for_task(task& desc);

	// searches for a task, and cancels it assumes m_lock is held
	bool priv_cancel_task(itask * const task);

	// waits for the task runner to signal that it's shutdown,
	// assumes m_shutdown_lock is held prior to being called
	void wait_for_task_runner();

	static void* task_runner(void* task_queue);
    };
}

#endif
