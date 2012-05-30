#ifndef TASK_QUEUE_HPP
#define TASK_QUEUE_HPP

#include <pthread.h>

#include "itask_queue.hpp"
#include "task_runner.hpp"

namespace libtq
{
    class itask;

    /** Queues and waits on individual tasks
     *
     *  This task queue allows threads to wait on arbitrary tasks
     *  without affecting the task runner thread.  Note that there is
     *  a little extra overhead to support waiting on arbitrary tasks.
     *
     *  @exception std::exception Unless specifically noted in a
     *  method's documentation, task_queue methods make no attempt to
     *  catch any exception thrown from the underlying STL container.
     *  So a std exception generated from an operation on said
     *  container will propagate to the caller.
     *
     *  @par Exception Safety:
     *  Unless specifically noted in a method's documentation, all methods
     *  have a strong exception guarantee in that if an exception is thrown,
     *  the task queue is in exactly the same state it was before the method
     *  was called.
     */
    class task_queue
    {
	public:

	task_queue();
	~task_queue();

	/** Starts the task queue's task runner thread
	 *
	 *  @return Zero if the task runner thread was successfully
	 *  started, non-zero otherwise.
	 */
	int start_queue();

	/** Shutdowns the task queue's task runner thread
	 *
	 *  This method blocks the calling thread until all the tasks
	 *  queued at the time it's called are finished, and the task
	 *  runner thread is stopped.
	 */
	void shutdown_queue();

	/** Stops the task runner thread
	 *
	 *  This method stops the task runner thread immediately.
	 *
	 *  @note After this method is finished there may still be
	 *  tasks queued.
	 *
	 *  @par Exception Safety:
         *  This method has a no-throw guarantee.
	 */
	void stop_queue();

	/** Cancels any tasks, and stops the runner thread
	 *
	 *  This method calls cancel_tasks and stop_queue atomically.
	 *
	 *  @par Exception Safety:
	 *  This method has a no-throw guarantee.
	 */
	void cancel_queue();

	/** Cancels all the queued tasks
	 *
	 *  @note This function does not stop the task runner thread.
	 */
	void cancel_tasks();

	/** Queues a task
	 *
	 *  A task can only be queued once.  So if the task was
	 *  already queued and is not yet complete, this function will
	 *  not queue the task a second time.
	 *
	 */
	void queue_task(itask * const task);

	/** Waits for a task to complete
	 *
	 *  The calling thread will block until the task execution
	 *  completes or the task is canceled.  If the task is not
	 *  already queued, this function exits immediately.
	 *
	 *  @return Greater than zero if the task was executed while
	 *  being waited on, less than zero if the task was canceled
	 *  while being waited on, or zero if the task was not already
	 *  scheduled.
	 *
	 *  @par Exception Safety:
	 *  This method has a no-throw guarantee.
	 */
	int wait_for_task(itask * const task);

	/** Cancels the execution of a task
	 *
	 *  Cancels a previously queued task, and notifies all the
	 *  threads waiting on the task that the task was canceled.
	 *
	 *  @return True if the task was canceled, false if the task
	 *  was not scheduled at the time this method was called.
	 *
	 *  @par Exception Safety:
	 *  This method has a no-throw guarantee.
	 */
	bool cancel_task(itask * const task);

	private:

	/// Predicate used to determine if the task runner thread is started
	bool m_started;

	/// Protects the m_started predicate.
	pthread_mutex_t m_shutdown_lock;

	/// Underlying task queue
	itask_queue m_queue;

	/// task runner object
	task_runner m_task_runner;

	/// assumes the m_shutdown_lock mutex is locked
	void locked_stop_queue();

	/// Copying is prohibited
	task_queue(const task_queue&);
	task_queue& operator =(const task_queue&);
    };

    inline void task_queue::queue_task(itask * const taskp)
    {
	m_queue.queue_task(taskp);
    }

    inline int task_queue::wait_for_task(itask * const taskp)
    {
	return m_queue.wait_for_task(taskp);
    }

    inline bool task_queue::cancel_task(itask * const taskp)
    {
	return m_queue.cancel_task(taskp);
    }
}

#endif
