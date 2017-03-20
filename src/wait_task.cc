#include <limits>
#include <stdexcept>
#include <mutex>
#include <condition_variable>

#include "wait_task.hpp"
#include "task_queue.hpp"

using namespace libtq;

wait_task::wait_task()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_counter = 0;
    m_scheduled = 0;
}

wait_task::~wait_task() {}

void wait_task::signal_waiters()
{
    {
	std::lock_guard<std::mutex> lock(m_mutex);
	if ( m_scheduled == 0 )
	{
	    return;
	}

	// Only increment m_counter if there are potentially threads
	// waiting for it to change.
	++m_counter;

	// Only decrement m_scheduled if the task is scheduled that
	// way m_scheduled does not wrap from 0 to -1.
	--m_scheduled;
    }

    m_cond.notify_all();
}

void wait_task::scheduled()
{
    // Let sub-classes know they've been scheduled
    wait_task_scheduled();

    std::lock_guard<std::mutex> lock(m_mutex);
    ++m_scheduled;
}

void wait_task::run()
{
    signaler signal(this);
    wait_task_run();
}

void wait_task::wait()
{
    counter_t counter;

    wait_task_wait();

    std::unique_lock<std::mutex> lock(m_mutex);

    if( m_scheduled == 0 )
    {
	// don't wait if the task is not scheduled
	return;
    }

    counter = m_counter;
    m_cond.wait(lock, [&] {return counter != m_counter;});
}

void wait_task::wait_task_run()
{}

void wait_task::wait_task_wait()
{}

void wait_task::wait_task_scheduled()
{}
