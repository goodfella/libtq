#ifndef WAIT_TASK_HPP
#define WAIT_TASK_HPP

#include <pthread.h>
#include "itask.hpp"

namespace libtq
{
    /// A class that implements wait capabilities
    /**
     *  The intention of this class is to be used as a base class for
     *  itasks that need to be waited on until they're ran.  Only the
     *  virtual methods of this class should be overriden.  The phrase
     *  "wait task object" will be used hereafter to refer to a
     *  wait_task object, or an object of a class which inherits from
     *  the wait_task class.
     *
     *  @par Copy Semantics:
     *  This class cannot be copied.
     *
     *  @par Thread Safety:
     *  All methods of this class are safe to call from multiple
     *  threads.  However, it is not thread safe to call a method on a
     *  wait task object in one thread while the same wait task object
     *  is being destroyed via its destructor in another thread.
     *
     *  @par Scheduling:
     *  A wait task object can be scheduled on multiple task queues.
     *  However, every thread blocked in the wait_task::wait method
     *  will be signaled from the first task_queue where the wait task
     *  object is ran.
     */
    class wait_task : public itask
    {
	public:

	wait_task();
	virtual ~wait_task();

	/// Signals all the waiting threads
	/**
	 *  @note Sub classes are not allowed to override this method.
	 *
	 *  @par Exception Safety
	 *  This method has a no throw exception guarantee
	 */
	void signal_waiters();

	/// Signals waiting threads
	/**
	 *  @note Sub classes are not allowed to override this method.
	 *  Instead, override wait_task::wait_task_run.
	 *
	 */
	void run();

	/// Signals waiting threads
	/**
	 *  @note Sub classes are not allowed to override this method.
	 *  Instead, override wait_task::wait_task_canceled.
	 *
	 *  @par Exception Safety
	 *  This method has a weak exception guarantee because waiting
	 *  threads are signaled even if wait_task::wait_task_canceled
	 *  throws an exception.
	 */
	void canceled();

	/// Called when the task is scheduled
	/**
	 *  @note Sub classes are not allowed to override this method.
	 *  Instead, override wait_task::wait_task_scheduled.
	 *
	 *  @par Exception Safety
	 *  This method has a strong exception guarantee.
	 */
	void scheduled();

	/// Blocks the calling thread until wait_task::run or wait_task::canceled is called
	/**
	 *  If the task is scheduled, then this method blocks until
	 *  the task is ran or canceled.  If the task is not
	 *  scheduled, then this method exits without blocking.
	 *
	 *  @note Sub classes are not allowed to override this method.
	 *  Instead override wait_task::wait_task_wait.
	 *
	 *  @par Exception Safety
	 *  This method has a strong exception guarantee.
	 */
	void wait();


	private:

	/// Signals a wait_task's waiters upon destruction
	class signaler
	{
	    public:

	    signaler(wait_task * const task): m_task(task) {}
	    ~signaler() {m_task->signal_waiters();}

	    private:

	    wait_task* m_task;
	};

	/// Called by wait_task::run
	/**
	 *  @see itask::run for restrictions when overriding this
	 *  method.
	 */
	virtual void wait_task_run();

	/// Called by wait_task::canceled
	/**
	 *  @see itask::canceled for restrictions when overriding this
	 *  method.
	 */
	virtual void wait_task_canceled();

	/// Called by wait_task::scheduled
	/**
	 *  @see itask::scheduled for restrictions when overriding this
	 *  method.
	 */
	virtual void wait_task_scheduled();

	/// Called by wait_task::wait
	virtual void wait_task_wait();

	// This class is not copyable
	wait_task(const wait_task& rhs);
	wait_task& operator =(const wait_task& rhs);

	typedef unsigned int counter_t;

	/// Used by wait_task::wait to know when the task has been ran
	counter_t m_counter;

	/// Used by wait_task::wait to know if the task is scheduled
	counter_t m_scheduled;

	pthread_mutex_t m_lock;
	pthread_cond_t m_cond;
    };
}

#endif
