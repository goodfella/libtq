#ifndef ITASK_QUEUE_HPP
#define ITASK_QUEUE_HPP

#include <pthread.h>
#include <list>

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

	/** Cancels the execution of a task
	 *
	 *  Cancels a previously queued task, and calls the canceled
	 *  method of the itask object.
	 *
	 *  @return true if the task was canceled, false if the task
	 *  was not scheduled at the time this method was called.
	 */
	bool cancel_task(itask * const task);

	/** Cancels all the tasks that are queued
	 *
	 *  @note This method notifies the waiters of each queued
	 *  task, that the task was canceled.
	 */
	void cancel_tasks();

	/** Runs the task at the front of the queue
	 *
	 *  Blocks if the queue is empty
	 */
	void run_task();

	/** Sets the cancel predicate
	 *
	 *  Calling this function will causes a canceled_runner
	 *  exception to be thrown in each thread inside run_task.
	 *  This behavior persists until clear_cancel is called.
	 */
	void set_cancel();

	/// Clears the cancel predicate
	void clear_cancel();

	private:

	// No copying allowed
	itask_queue(const itask_queue& rhs);
	itask_queue& operator=(const itask_queue& rhs);

	/// Set to true when threads in run_task should be canceled
	bool m_cancel;

	/// List of tasks to run
	std::list<itask*> m_tasks;

	/// Protects the m_tasks list and m_cancel
	pthread_mutex_t m_lock;

	/// Used to signal a thread waiting on an empty task list
	pthread_cond_t m_cond;
    };
}

#endif
