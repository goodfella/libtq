#include <limits>
#include <stdexcept>

#include "wait_task.hpp"
#include "mutex_lock.hpp"

using namespace libtq;

wait_task::wait_task()
{
    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_cond, NULL);

    mutex_lock lock(&m_lock);

    m_counter = 0;
    m_scheduled = 0;
}

wait_task::~wait_task()
{
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_lock);
}

void wait_task::signal_waiters()
{
    mutex_lock lock(&m_lock);

    ++m_counter;

    if( m_scheduled > 0 )
    {
	--m_scheduled;
    }

    pthread_cond_broadcast(&m_cond);
}

void wait_task::run()
{
    wait_task_run();
    signal_waiters();
}

void wait_task::canceled()
{
    // signal waiters after calling wait_task_canceled and even if
    // wait_task_canceled throws an exception.  signal_waiters is
    // called after wait_task_canceled so that the cancelation
    // routines are completed before waiting threads are signaled.
    wait_task::signaler cleanup(this);
    wait_task_canceled();
}

void wait_task::scheduled()
{
    {
	mutex_lock lock(&m_lock);

	if( m_scheduled == std::numeric_limits<counter_t>::max() )
	{
	    // scheduling more than the max value of counter_t is not allowed 
	    throw std::overflow_error("wait_task scheduled too many times");
	}

	++m_scheduled;
    }

    try
    {
	// call wait_task_scheduled without m_lock held.  If
	// wait_task_scheduled throws an exception, then decrement
	// m_scheduled and re-throw the exception.
	wait_task_scheduled();
    }
    catch(...)
    {
	mutex_lock lock(&m_lock);
	--m_scheduled;
	throw;
    }
}

void wait_task::wait()
{
    counter_t counter;

    wait_task_wait();

    mutex_lock lock(&m_lock);

    if( m_scheduled == 0 )
    {
	// don't wait if the task is not scheduled
	return;
    }

    counter = m_counter;

    // Wait until the counters are not equal which means the task was ran
    while( counter == m_counter )
    {
	pthread_cond_wait(&m_cond, &m_lock);
    }
}

void wait_task::wait_task_run()
{}

void wait_task::wait_task_canceled()
{}

void wait_task::wait_task_scheduled()
{}

void wait_task::wait_task_wait()
{}
