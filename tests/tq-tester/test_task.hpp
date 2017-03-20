#ifndef TEST_TASK_HPP
#define TEST_TASK_HPP

#include <mutex>
#include "wait_task.hpp"

namespace tq_tester
{
    class test_task : public libtq::wait_task
    {
	public:
	
	test_task();

	const unsigned long runcount() const;
	const unsigned long waitcount() const;

	private:

	void wait_task_run() override;
	void wait_task_wait() override;
	
	void inc_counter(unsigned long& counter);
	const unsigned long counter(const unsigned long& counter) const;
	
	mutable std::mutex m_mutex;
	unsigned long m_runcount;
	unsigned long m_waitcount;
    };
}

#endif
