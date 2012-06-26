#ifndef WAIT_TASK_HPP
#define WAIT_TASK_HPP

#include <pthread.h>
#include "itask.hpp"

namespace libtq
{
    /// A class that implements wait capabilities
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
	virtual void wait_task_canceled();

	/// Called by wait_task::scheduled
	virtual void wait_task_scheduled();

	/// Called by wait_task::wait
	virtual void wait_task_wait();

	// This class is not copyable
	wait_task(const wait_task& rhs);
	wait_task& operator =(const wait_task& rhs);

	// Used by the waiters to know when the task has been ran
	int m_counter;
	bool m_scheduled;

	pthread_mutex_t m_lock;
	pthread_cond_t m_cond;
    };
}

#endif
