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
	~wait_task();

	/// Signals all the waiting threads
	/**
	 *  @note Classes that override this method should call
	 *  wait_task::signal_waiters in their version to insure the
	 *  waiters are signaled.
	 */
	virtual void signal_waiters();

	/// Signals waiting threads
	/**
	 *  @note Child classes that override this method should call
	 *  signal_waiters or use the signaler class.
	 */
	virtual void run();

	/// Signals waiting threads
	/**
	 *  @note Child classes that override this method should call
	 *  signal_waiters or use the signaler class.
	 */
	virtual void canceled();

	/// Called when the task is scheduled
	/**
	 *  @note Child classes that override this method need to call
	 *  wait_task::scheduled at some point in their version of it.
	 */
	virtual void scheduled();

	/// Waits until either wait_task::run or wait_task::canceled is called
	/**
	 *  If the task is scheduled, then this method blocks until
	 *  the task is ran or canceled.  If the task is not
	 *  scheduled, then this method exits without blocking.
	 *
	 *  @note Classes that override this method need to call
	 *  wait_task::wait in order to wait on the task.
	 */
	virtual void wait();

	/// Signals a wait_task's waiters upon destruction
	class signaler
	{
	    public:

	    signaler(wait_task * const task): m_task(task) {}
	    ~signaler() {m_task->signal_waiters();}

	    private:

	    wait_task* m_task;
	};

	private:

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
