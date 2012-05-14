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
    class task_desc
    {
	public:

	task_desc();
	explicit task_desc(itask* task);

	/* Copies m_task from rhs, makes adds rhs.m_waitlist to
	 * m_waitlist, and removes rhs.m_waitlist from the waitlist */
	void move(task_desc& rhs);

	void signal_finished();
	void run_task();

	void add_to_waitlist(wait_desc* desc);
	void detach_listhead();

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
