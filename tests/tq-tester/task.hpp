#ifndef TASK_HPP
#define TASK_HPP

#include <pthread.h>
#include "itask.hpp"

namespace tq_tester
{
    class task : public libtq::itask
    {
	public:
	
	task();
	~task();
	
	void run();
	const unsigned long runcount() const;
	const unsigned long cancelcount() const;
	void inc_waitcount();
	void inc_cancelcount();
	
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