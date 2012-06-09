#include <algorithm>
#include <functional>
#include <vector>

#include "itask_queue.hpp"
#include "itask.hpp"
#include "mutex_lock.hpp"
#include "runner_canceled.hpp"

using namespace std;
using namespace libtq;

itask_queue::itask_queue():
    m_cancel(false),
    m_canceling(false)
{
    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_cond, NULL);
    pthread_cond_init(&m_cancel_cond, NULL);
}

itask_queue::~itask_queue()
{
    pthread_cond_destroy(&m_cancel_cond);
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
	    m_tasks.push_back(itaskp);

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
    // Used to determine whether or not itaskp needs to be canceled
    bool ret = false;

    {
	mutex_lock lock(&m_lock);

	// Only one thread at a time is allowed to cancel a task.  A
	// condition is used here because itask::canceled should be
	// called without any locks held
	while( m_canceling == true )
	{
	    pthread_cond_wait(&m_cancel_cond, &m_lock);
	}

	// This thread is now canceling tasks, so set m_canceling so
	// other threads block until they're signaled
	m_canceling = true;

	if( m_tasks.empty() == true )
	{
	    // The task queue is empty, so there's no task to cancel
	    ret = false;
	}
	else
	{
	    // Search for the task to cancel
	    list<itask*>::iterator th = find(m_tasks.begin(), m_tasks.end(), itaskp);

	    if( th != m_tasks.end() )
	    {
		m_tasks.erase(th);
		ret = true;
	    }
	}
    }

    if( ret == true )
    {
	// itaskp was in the queue, so cancel it
	itaskp->canceled();
    }

    {
	// Clear m_canceling so another thread can cancel tasks
	mutex_lock lock(&m_lock);
	m_canceling = false;
    }

    // signal only one thread because only one thread at a time is
    // allowed to cancel tasks
    pthread_cond_signal(&m_cancel_cond);

    return ret;
}

void itask_queue::cancel_tasks()
{
    vector<itask*> tasks;

    {
	mutex_lock lock(&m_lock);

	// Wait until no other thread is canceling a task
	while( m_canceling == true )
	{
	    pthread_cond_wait(&m_cancel_cond, &m_lock);
	}

	m_canceling = true;

	// resize can throw an exception, so catch it to set
	// m_canceling back to false
	try
	{
	    tasks.resize(m_tasks.size());
	}
	catch(std::exception& ex)
	{
	    m_canceling = false;
	    throw;
	}

	// Copy the tasks so they can be canceled and clear the
	// original task list
	copy(m_tasks.begin(), m_tasks.end(), tasks.begin());
	m_tasks.clear();
    }

    // Cancel each task
    for_each(tasks.begin(), tasks.end(), mem_fun(&itask::canceled));

    {
	// clear m_canceling so other threads can cancel tasks
	mutex_lock lock(&m_lock);
	m_canceling = false;
    }

    // signal one thread waiting to cancel tasks
    pthread_cond_signal(&m_cancel_cond);
}

void itask_queue::set_cancel()
{
    {
	mutex_lock lock(&m_lock);
	m_cancel = true;
    }

    // Wake up any waiting task runners
    pthread_cond_broadcast(&m_cond);
}

void itask_queue::clear_cancel()
{
    mutex_lock lock(&m_lock);
    m_cancel = false;
}

void itask_queue::run_task()
{
    itask* task = NULL;

    {
	mutex_lock lock(&m_lock);

	if( m_cancel == true )
	{
	    throw runner_canceled();
	}

	while( m_tasks.empty() == true )
	{
	    // wait until the queue is not empty
	    pthread_cond_wait(&m_cond, &m_lock);

	    if( m_cancel == true )
	    {
		throw runner_canceled();
	    }
	}

	// At this point the queue is not empty, and we have the lock
	// from either the mutex_lock at the top, or from the
	// pthread_cond_wait in the while loop

	// Get a copy of the task pointer and remove the task from the
	// task list
	task = m_tasks.front();
	m_tasks.pop_front();
    }
    
    task->run();
}
