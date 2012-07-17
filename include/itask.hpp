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
    };
}

#endif

