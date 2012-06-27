#ifndef ITASK_H
#define ITASK_H

namespace libtq
{
    /// Base class for all tasks that are ran on the queue
    /**
     *  Classes that inherit from this class can be scheduled as tasks
     *  on the task_queue.
     */
    class itask
    {
	public:

	virtual ~itask();

	/** Called by the task_queue to perform the task's actions
	 *
	 *  @par Restrictions:
	 *  The task_queue::shutdown_queue, task_queue::stop_queue,
	 *  and task_queue::cancel_queue methods cannot be called from
	 *  this method.  Also, no exception is allowed to leave this
	 *  method.
	 */
	virtual void run() = 0;

	/**
	 *  @brief Called by the task_queue when the task is canceled
	 *  and will not be ran.
	 *
	 *  @par Restrictions:
	 *  No task_queue method can be called from this method.
	 */ 
	virtual void canceled();

	/// Called when the task is scheduled on the queue
	/**
	 *  @note This method is only called once per run.  So if the
	 *  task has been scheduled, but not yet ran, and the task is
	 *  scheduled again, this method is not called again.
	 *
	 *  @par Restrictions:
	 *  No task_queue methods can be called from this method.
	 */
	virtual void scheduled();
    };
}

#endif

