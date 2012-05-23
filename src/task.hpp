#ifndef TASK_HPP
#define TASK_HPP

#include <pthread.h>

namespace libtq
{
    class itask;

    /** Keeps track of all the waiters
     */
    class task
    {
	friend class task_allocator;

	public:

	task();
	~task();

	/// Increments the ref count of the task
	void get_ref();

	/// Decrements the ref count of the task and returns the new ref count value
	int put_ref();

	/// Signals the waiters that the task is finished
	void signal_finished();

	/// signals the waiters that the task has been canceled
	void signal_canceled();

	/// Runs the task
	void run_task() const;

	/// Waits for the task
	const bool wait_for_task();

	/// Returns true if the object refers to the given itask
	const bool operator==(itask const * const task) const;

	private:

	// used by the allocator, so no one else should use them

	// sets the itask pointer and returns the next pointer
	task* const itaskp(itask * const itaskp);

	// resets the values and sets the next pointer
	void reset(task * const next);

	// purposely not defined because they should never be called
	task& operator=(const task& rhs);
	task(const task& rhs);

	// this gives the allocator the means to store tasks in a
	// singly linked list without using up more memory and without
	// having to deal with alignment issues
	union
	{
	    itask* m_task;
	    task* m_next;
	};

	bool m_finished;
	bool m_canceled;
	int m_refcount;

	/// Locks the m_finished and m_canceled booleans
	mutable pthread_mutex_t m_state_lock;

	/** Locks the m_task and m_next pointers
	 *
	 * A rw lock is used here because most of the time, the
	 * pointers are read from and not written to.
	 */
	mutable pthread_rwlock_t m_task_lock;

	/** Locks the ref count
	 *
	 *  This should be unnecessary, but right now there's no
	 *  atomic support in C++.  Although I may end up using GCC's
	 *  __sync_add_and_fetch builtin in the interim.
	 */
	pthread_mutex_t m_ref_lock;

	/// Condition used to signal waiters when the task is complete or canceled
	pthread_cond_t m_cond;
    };
}

#endif
