#ifndef TEST_TASK_HPP
#define TEST_TASK_HPP

#include <pthread.h>
#include "itask.hpp"

namespace tq_tester
{
    class test_task : public libtq::itask
    {
	public:
	
	test_task();
	~test_task();
	
	void run();
	void canceled();
	const unsigned long runcount() const;
	const unsigned long cancelcount() const;
	const unsigned long waitcount() const;
	void inc_waitcount();

	private:
	
	void inc_runcount();
	void inc_counter(unsigned long& counter);
	const unsigned long counter(const unsigned long& counter) const;
	
	mutable pthread_mutex_t m_lock;
	unsigned long m_runcount;
	unsigned long m_waitcount;
	unsigned long m_cancelcount;
    };
}

#endif
