#ifndef ITASK_H
#define ITASK_H

namespace libtq
{
    /// Base class for all tasks that are ran on the queue
    /**
     *  Classes that inherit from this class can be scheduled as tasks
     *  on the task_queue and the itask::run method will be invoked
     *  when the task_queue runs each task.  The itask::canceled
     *  method is called by the task_queue when a task is canceled.
     *
     *  @note The task_queue::shutdown_queue, task_queue::stop_queue,
     *  and task_queue::cancel_queue methods cannot be called from the
     *  itask::run method.  The task_queue::cancel_tasks and
     *  task_queue::cancel_task methods cannot be called from the
     *  itask::canceled method.  Also, exceptions are not allowed to
     *  leave itask::run or itask::canceled.
     *
     */
    class itask
    {
	public:

	virtual ~itask();

	/**
	 *  @note The task_queue::shutdown_queue,
	 *  task_queue::stop_queue, and task_queue::cancel_queue
	 *  cannot be called from this method.  This method must catch
	 *  all exceptions because no exception is allowed to leave
	 *  this method.
	 */
	virtual void run() = 0;

	/** 
	 *  @note The task_queue::cancel_tasks, and
	 *  task_queue::cancel_task cannot be called from this method.
	 *  This method must catch all exceptions because no exception
	 *  is allowed to leave this method.
	 */ 
	virtual void canceled();
    };
}

#endif

