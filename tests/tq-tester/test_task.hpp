#ifndef TEST_TASK_HPP
#define TEST_TASK_HPP

#include <pthread.h>
#include "wait_task.hpp"

namespace tq_tester
{
    class test_task : public libtq::wait_task
    {
	public:
	
	test_task();
	~test_task();
	
	const unsigned long runcount() const;
	const unsigned long waitcount() const;

	private:

	void wait_task_run();
	void wait_task_wait();
	
	void inc_counter(unsigned long& counter);
	const unsigned long counter(const unsigned long& counter) const;
	
	mutable pthread_mutex_t m_lock;
	unsigned long m_runcount;
	unsigned long m_waitcount;
    };
}

#endif
