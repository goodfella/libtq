#ifndef TASK_DESC_HPP
#define TASK_DESC_HPP

#include <pthread.h>

namespace libtq
{
    class itask;

    class wait_desc
    {
	public:

	explicit wait_desc(pthread_mutex_t* m);
	wait_desc();
	wait_desc(const wait_desc& wd);

	~wait_desc();

	void add_to_waitlist(wait_desc* waitlist);
	void wait_for_task();
	void signal_waiters();

	private:

	void remove_from_waitlist();

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
