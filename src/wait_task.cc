#include "wait_task.hpp"
#include "mutex_lock.hpp"

using namespace libtq;

wait_task::wait_task()
{
    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_cond, NULL);

    mutex_lock lock(&m_lock);

    m_counter = 0;
    m_scheduled = false;
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
    m_scheduled = false;

    pthread_cond_broadcast(&m_cond);
}

void wait_task::run()
{
    wait_task_run();
    signal_waiters();
}

void wait_task::canceled()
{
    wait_task_canceled();

    // signal the waiting threads
    signal_waiters();
}

void wait_task::scheduled()
{
    wait_task_scheduled();

    mutex_lock lock(&m_lock);
    m_scheduled = true;
}

void wait_task::wait()
{
    int counter;

    wait_task_wait();

    mutex_lock lock(&m_lock);

    if( m_scheduled == false )
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
