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
	 *
	 *  @return zero if the task runner thread was successfully
	 *  shutdown, non-zero if it was not shut down.
	 */
	int stop_queue();

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
	 *  @return zero if the wait was succesfull, or if the task
	 *  was not already queued.  Non-zero, if the task was queued,
	 *  and the wait failed.
	 */
	int wait_for_task(itask * const task);

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
	 *  @param cancel_status Set to true if the task was canceled
	 *  and will not execute.  Set to false if the task was not
	 *  canceled i.e. the task runner is preparing to run the
	 *  task.
	 *
	 *  @return Zero if the required wait, due to the task runner
	 *  preparing to run the task succeeded, or if the task runner
	 *  was not preparing to run the task.  Non-zero if the
	 *  required wait, due to the task runner preparing to run the
	 *  task, failed.
	 */
	int cancel_task(itask * const task, bool& cancel_status);
	int cancel_task(itask * const task);

	private:

	// Predicate used to determine if the task runner thread is
	bool m_started;

	// List of tasks to run
	std::list<task_desc> m_tasks;

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
	 *  already queued, then it's not waited on.  So callers
	 *  should make sure the task is not already queued prior to
	 *  calling this function otherwise, the return value is
	 *  ambiguous.
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
	 *  @note Assumes m_lock is held prior to being called
	 */
	bool priv_queue_task(itask * const task);

	// assumes m_lock is held prior to being called
	int priv_wait_for_task(itask* const task);

	// assumes m_lock is held prior to being called
	int priv_wait_for_task(task_desc& desc);

	int cancel_task(itask * const task, bool* cancel_status);

	// searches for a task, and cancels it assumes m_lock is held
	void priv_cancel_task(itask * const task, bool* cancel_status);

	static void* task_runner(void* task_queue);
    };
}

#endif
