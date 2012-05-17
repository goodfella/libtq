#include <algorithm>

#include "itask_queue.hpp"
#include "task.hpp"
#include "itask.hpp"
#include "task_cleanup.hpp"
#include "mutex_lock.hpp"

using namespace std;
using namespace libtq;

itask_queue::itask_queue()
{
    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_cond, NULL);
}

itask_queue::~itask_queue()
{
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_lock);
}

void itask_queue::queue_task(itask * const itaskp)
{
    mutex_lock lock(&m_lock);

    if( find(m_tasks.begin(), m_tasks.end(), itaskp) == m_tasks.end() )
    {
	// The task is not already scheduled
	m_tasks.push_back(task_handle(m_allocator.alloc(itaskp), &m_allocator));

	// task queue was empty signal the task runner
	if( m_tasks.size() == 1 )
	{
	    pthread_cond_signal(&m_cond);
	}
    }
}

bool itask_queue::cancel_task(itask * const itaskp)
{
    task_handle thandle;

    {
	// we have a separate scope here so the lock is unlocked when
	// the waiters are signaled
	mutex_lock lock(&m_lock);

	if( m_tasks.empty() == true )
	{
	    // the task queue is empty, so there's no task to cancel
	    return false;
	}

	list<task_handle>::iterator th = find(m_tasks.begin(), m_tasks.end(), itaskp);

	if( th != m_tasks.end() )
	{
	    thandle = *th;
	    m_tasks.erase(th);
	}
    }

    if( thandle.is_set() )
    {
	thandle->signal_finished();
	return true;
    }

    return false;
}

bool itask_queue::wait_for_task(itask * const taskp)
{
    task_handle thandle;

    {
	mutex_lock lock(&m_lock);

	list<task_handle>::iterator th = find(m_tasks.begin(), m_tasks.end(), taskp);

	if( th != m_tasks.end() )
	{
	    thandle = *th;
	}
    }

    if( thandle.is_set() == true )
    {
	thandle->wait_for_task();
	return true;
    }

    return false;
}

void itask_queue::run_task()
{
    mutex_lock lock(&m_lock);

    while( m_tasks.empty() == true )
    {
	// wait until the queue is not empty
	pthread_cond_wait(&m_cond, &m_lock);
    }

    // at this point the queue is not empty, and we have the lock from
    // either the mutex_lock at the top, or from the pthread_cond_wait
    // in the while loop

    // take ownership of the task and remove the old task from the
    // task list
    task_cleanup tcleanup(m_tasks.front());

    m_tasks.pop_front();
    
    // unlock the lock, and run the task
    lock.unlock();
    tcleanup->run_task();
}
