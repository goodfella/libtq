#ifndef ITASK_QUEUE_HPP
#define ITASK_QUEUE_HPP

#include <pthread.h>
#include <list>

#include "task_handle.hpp"
#include "task_allocator.hpp"

namespace libtq
{
    class itask;

    /// Represents a queue of itasks
    class itask_queue
    {
	public:

	itask_queue();

	/** Destroys the itask_queue
	 *
	 *  @note Having threads blocking in run_task while this
	 *  destructor is ran, or having tasks that are still queued,
	 *  is undefined.
	 */
	~itask_queue();

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

	/** Runs the task at the front of the queue
	 *
	 *  Blocks if the queue is empty
	 */
	void run_task();

	private:

	/// List of tasks to run
	std::list<task_handle> m_tasks;

	/// Protects the m_tasks list.
	pthread_mutex_t m_lock;

	/// Used to signal a thread waiting on an empty task list
	pthread_cond_t m_cond;

	/// allocator for task objects
	task_allocator m_allocator;
    };
}

#endif
