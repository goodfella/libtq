#ifndef TASK_DESC_HPP
#define TASK_DESC_HPP

#include <pthread.h>

namespace libtq
{
    class itask;

    struct wait_desc
    {
	wait_desc(pthread_mutex_t* m, pthread_cond_t* cond, bool* finished);

	void add_to_waitlist(wait_desc* waitlist);
	void remove_from_waitlist();

	pthread_mutex_t* mutex;
	pthread_cond_t* condition;
	bool* finished;

	struct wait_desc* next;
	struct wait_desc* prev;
    };

    class task_desc
    {
	public:

	task_desc(itask* task);

	int wait_for_task(pthread_mutex_t* lock);
	void signal_finished();
	void run_task();

	const bool operator==(itask * const task) const;

	private:

	void add_to_waitlist(wait_desc* desc);
	static void cleanup_waiter(void* waiter);

	itask* m_task;
	wait_desc* m_waitlist;
    };

    inline const bool task_desc::operator==(itask * const task) const
    {
	return task == m_task;
    }
}

#endif
