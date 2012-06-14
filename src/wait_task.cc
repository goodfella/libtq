#include "wait_task.hpp"
#include "mutex_lock.hpp"

using namespace libtq;

wait_task::cleanup::cleanup(wait_task * const wt):
    m_wait_task(wt)
{}

wait_task::cleanup::~cleanup()
{
    m_wait_task->run();
}

wait_task::cleanup::cleanup(const wait_task::cleanup& rhs):
    m_wait_task(rhs.m_wait_task)
{}

wait_task::cleanup& wait_task::cleanup::operator= (const wait_task::cleanup& rhs)
{
    if( &rhs == this )
    {
	return *this;
    }

    m_wait_task = rhs.m_wait_task;

    return *this;
}

wait_task::wait_task():
    m_counter(0),
    m_scheduled(false)
{
    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_cond, NULL);
}

wait_task::~wait_task()
{
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_lock);
}

void wait_task::run()
{
    mutex_lock lock(&m_lock);

    ++m_counter;
    m_scheduled = false;

    pthread_cond_broadcast(&m_cond);
}

void wait_task::canceled()
{
    // signal the waiting threads
    run();
}

void wait_task::scheduled()
{
    mutex_lock lock(&m_lock);
    m_scheduled = true;
}

void wait_task::wait()
{
    int counter;

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
