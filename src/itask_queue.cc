#include <algorithm>
#include <functional>
#include <vector>

#include "itask_queue.hpp"
#include "itask.hpp"
#include "mutex_lock.hpp"
#include "runner_canceled.hpp"

using namespace std;
using namespace libtq;

itask_queue::itask_queue()
{
    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_cond, NULL);

    mutex_lock lock(&m_lock);

    m_cancel = false;
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
	    // Schedule the task after calling itask::scheduled() to
	    // maintain the strong exception guarantee
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
