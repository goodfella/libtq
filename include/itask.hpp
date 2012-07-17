#ifndef ITASK_H
#define ITASK_H

namespace libtq
{
    class itask_queue;

    /// Base class for all tasks that are ran on the queue
    /**
     *  Classes that inherit from this class can be scheduled as tasks
     *  on the task_queue.
     */
    class itask
    {
	friend class itask_queue;

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

	private:

	/// Called when the task is scheduled on a task_queue
	/**
	 *  @note This method is invoked every time an itask is
	 *  scheduled on a given task_queue.  However, if the itask is
	 *  pending on a given task_queue, and scheduled again on the
	 *  that task_queue, then this method is not invoked.
	 *
	 *  @par Restrictions:
	 *  No task_queue methods can be called from this method.
	 */
	virtual void scheduled();
    };
}

#endif

