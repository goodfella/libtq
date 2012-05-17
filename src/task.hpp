#ifndef TASK_HPP
#define TASK_HPP

#include <pthread.h>

namespace libtq
{
    class itask;
    class task_allocator;

    class wait_desc
    {
	public:

	explicit wait_desc(pthread_mutex_t* m);
	wait_desc();
	wait_desc(const wait_desc& wd);

	~wait_desc();

	void add_to_waitlist(wait_desc* waitlist);
	void remove_from_waitlist();
	void replace_head(wait_desc* const new_head);

	void wait_for_task(pthread_mutex_t* mutex);
	void signal_waiters();

	private:

	// purposely not defined because it should never be used
	wait_desc& operator=(const wait_desc& rhs);

	struct wait_desc* m_next;
	struct wait_desc* m_prev;

	bool m_finished;
	pthread_cond_t* m_condition;
    };

    /** Keeps track of all the waiters
     */
    class task
    {
	friend class task_allocator;

	public:

	task();

	/// Increments the ref count of the task
	void get_ref();

	/// Decrements the ref count of the task and returns the new ref count value
	int put_ref();

	/// Signals the waiters that the task is finished
	void signal_finished();

	/// Runs the task
	void run_task();

	/// Waits for the task
	void wait_for_task();

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
	int ref_count;

	mutable pthread_mutex_t m_lock;
	pthread_mutex_t m_ref_lock;
	pthread_cond_t m_cond;
    };
}

#endif
