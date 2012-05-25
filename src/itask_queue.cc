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
    bool signal = false;

    {
	/* There's a separate scope here so the signal can be done
	 * without holding the mutex. */

	mutex_lock lock(&m_lock);

	if( find(m_tasks.begin(), m_tasks.end(), itaskp) == m_tasks.end() )
	{
	    // The task is not already scheduled
	    m_tasks.push_back(task_handle(m_allocator.alloc(itaskp), &m_allocator));

	    // task queue was empty signal the task runner
	    if( m_tasks.size() == 1 )
	    {
		signal = true;
	    }
	}
    }

    if( signal == true )
    {
	/* Signal a waiting thread if the queue was previously empty */
	pthread_cond_signal(&m_cond);
    }
}

bool itask_queue::cancel_task(itask * const itaskp)
{
    task_cleanup tcleanup(&task::signal_canceled);
    bool ret = false;

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
	    tcleanup = *th;
	    m_tasks.erase(th);
	    ret = true;
	}
    }

    return ret;
}

void itask_queue::cancel_tasks()
{
    mutex_lock lock(&m_lock);

    while( m_tasks.empty() == false )
    {
	task_cleanup cleanup_task(m_tasks.front(), &task::signal_canceled);
	m_tasks.pop_front();
    }
}

int itask_queue::wait_for_task(itask * const taskp)
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
	return (thandle->wait_for_task() == true ? 1 : -1);
    }

    return 0;
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
    task_cleanup tcleanup(m_tasks.front(), &task::signal_finished);

    m_tasks.pop_front();
    
    // unlock the lock, and run the task
    lock.unlock();
    tcleanup->run_task();
}
