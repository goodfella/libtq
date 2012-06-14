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

	class cleanup
	{
	    friend class wait_task;

	    public:

	    ~cleanup();
	    cleanup(const cleanup& rhs);
	    cleanup& operator= (const cleanup& rhs);

	    private:

	    cleanup(wait_task* wt);

	    wait_task* m_wait_task;
	};

	wait_task();
	~wait_task();

	/// Signals waiting threads
	virtual void run();

	/// Signals waiting threads
	virtual void canceled();

	/// Called when the task is scheduled
	/**
	 *  @note Classes that override this method need to call
	 *  wait_task::scheduled at some point in their version of it.
	 */
	virtual void scheduled();

	/// Waits until either wait_task::run or wait_task::canceled is called
	/**
	 *  @note Classes that override this method need to call
	 *  wait_task::wait in order to wait on the task.
	 */
	virtual void wait();

	/// Returns a wait_task::cleanup
	/**
	 *  This method provides derived classes with an object that
	 *  can be created on the stack, and will signal waiting
	 *  threads when it's destroyed.
	 */
	const wait_task::cleanup& get_cleanup() const;

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
