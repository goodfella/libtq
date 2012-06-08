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
     *  The itask::run and itask::canceled methods are called without
     *  any locks held.
     *
     *  @note The task_queue::shutdown_queue method cannot be called
     *  from either itask::run or itask::canceled.
     *
     */
    class itask
    {
	public:

	virtual ~itask();

	/// @note calling the task_queue::shutdown_queue method is undefined
	virtual void run() = 0;

	/// @note calling the task_queue::shutdown_queue method is undefined
	virtual void canceled();
    };
}

#endif

