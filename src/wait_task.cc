#include <limits>
#include <stdexcept>

#include "wait_task.hpp"
#include "mutex_lock.hpp"
#include "task_queue.hpp"

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

    if( m_scheduled > 0 )
    {
	// Only increment m_counter if there are potentially threads
	// waiting for it to change.
	++m_counter;

	// Only decrement m_scheduled if the task is scheduled that
	// way m_scheduled does not wrap from 0 - 1.
	--m_scheduled;

	// No need to broadcast the condition if there are no blocked
	// threads.  When m_scheduled == 0, wait_task::wait does not
	// block.
	pthread_cond_broadcast(&m_cond);
    }
}

void wait_task::schedule(task_queue * const queue)
{
    // Let sub-classes know they've been scheduled
    wait_task_scheduled();

    mutex_lock lock(&m_lock);

    queue->queue_task(this);
    ++m_scheduled;
}

void wait_task::run()
{
    /* Handling exceptions here is unecessary because exceptions are
     * not allowed to leave itask::run */

    wait_task_run();
    signal_waiters();
}

bool wait_task::wait()
{
    counter_t counter;

    wait_task_wait();

    mutex_lock lock(&m_lock);

    if( m_scheduled == 0 )
    {
	// don't wait if the task is not scheduled
	return false;
    }

    counter = m_counter;

    // Wait until the counters are not equal which means the task was ran
    while( counter == m_counter )
    {
	pthread_cond_wait(&m_cond, &m_lock);
    }

    return true;
}

void wait_task::wait_task_run()
{}

void wait_task::wait_task_wait()
{}

void wait_task::wait_task_scheduled()
{}
