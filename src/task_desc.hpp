#ifndef TASK_DESC_HPP
#define TASK_DESC_HPP

#include <pthread.h>

namespace libtq
{
    class itask;
    class task_desc;

    class wait_desc
    {
	friend class task_desc;

	public:

	wait_desc(pthread_mutex_t* m);
	wait_desc();
	~wait_desc();

	void wait_for_task();

	private:

	void add_to_waitlist(wait_desc* waitlist);
	void remove_from_waitlist();
	wait_desc(const wait_desc& wd);

	// purposely not defined because it should never be used
	wait_desc& operator=(const wait_desc& rhs);

	pthread_mutex_t* mutex;
	struct wait_desc* next;
	struct wait_desc* prev;

	bool finished;
	pthread_cond_t condition;
    };

    /** Keeps track of all the waiters
     */
    class task_desc
    {
	public:

	task_desc(itask* task);

	void wait_for_task(pthread_mutex_t* lock);
	void signal_finished();
	void run_task();

	void add_to_waitlist(wait_desc* desc);

	const bool operator==(itask * const task) const;

	private:

	// purposely not defined because it should never be called
	task_desc& operator=(const task_desc& rhs);

	itask* m_task;
	wait_desc m_waitlist;
    };

    inline const bool task_desc::operator==(itask * const task) const
    {
	return task == m_task;
    }
}

#endif
