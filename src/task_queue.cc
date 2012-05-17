#include <signal.h>
#include <algorithm>
#include <iostream>

#include "task_queue.hpp"
#include "itask.hpp"
#include "mutex_lock.hpp"
#include "task.hpp"
#include "task_cleanup.hpp"

using namespace std;
using namespace libtq;

namespace
{
    struct shutdown_exception {};

    struct shutdown_task : public itask
    {
	void run();
    };

    void shutdown_task::run()
    {
	throw shutdown_exception();
    }
}

task_queue::task_queue():
    m_started(false)
{
    pthread_mutex_init(&m_lock, NULL);
    pthread_mutex_init(&m_shutdown_lock, NULL);
    pthread_cond_init(&m_cond, NULL);
}

task_queue::~task_queue()
{
    stop_queue();
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_shutdown_lock);
    pthread_mutex_destroy(&m_lock);
}

int task_queue::start_queue()
{
    mutex_lock lock(&m_shutdown_lock);

    if( m_started == true )
    {
	return 0;
    }

    int start_ret = pthread_create(&m_thread, NULL, task_runner, this);
    m_started = (start_ret == 0 ? true : false);

    return start_ret;
}

void task_queue::stop_queue()
{
    mutex_lock shutdown_lock(&m_shutdown_lock);

    if( m_started == false )
    {
	return;
    }

    shutdown_task kill_task_runner;

    // queue and wait for the shutdown task to finish
    queue_task(&kill_task_runner);
    wait_for_task(&kill_task_runner);

    pthread_join(m_thread, NULL);

    m_started = false;
}

bool task_queue::priv_queue_task(itask * const itaskp)
{
    // a task can only be scheduled once
    if( find(m_tasks.begin(), m_tasks.end(), itaskp) == m_tasks.end() )
    {
	m_tasks.push_back(task_handle(m_allocator.alloc(itaskp), &m_allocator));

	// task queue was empty signal the task runner
	if( m_tasks.size() == 1 )
	{
	    pthread_cond_signal(&m_cond);
	}

	return true;
    }

    return false;
}

void task_queue::queue_task(itask * const task)
{
    mutex_lock lock(&m_lock);
    priv_queue_task(task);
}

bool task_queue::cancel_task(itask * const itaskp)
{
    task_handle thandle;

    {
	// we have separate scope here so the lock is unlocked when
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

bool task_queue::wait_for_task(itask * const taskp)
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

void* task_queue::task_runner(void* tqueue)
{
    task_queue * queue = static_cast<task_queue*>(tqueue);

    pthread_mutex_lock(&queue->m_lock);

    do
    {
	/* At this if statement, the lock is taken from one of three
	 * places
	 *
	 * From the pthread_mutex_lock at the top of this function.
	 *
	 * From the pthread_mutex_lock after the catch statement.
	 *
	 * From the return of pthread_cond_wait in the else statement.
	 */
	if( queue->m_tasks.empty() == false )
	{
	    try
	    {
		task_cleanup tcleanup;

		// take ownership of the task and remove the old
		// task from the task list
		tcleanup = queue->m_tasks.front();
		queue->m_tasks.pop_front();

		// run the task
		pthread_mutex_unlock(&queue->m_lock);
		tcleanup->run_task();
	    }
	    catch (const shutdown_exception& ex)
	    {
		// we need to shutdown
		return NULL;
	    }

	    pthread_yield();
	    pthread_mutex_lock(&queue->m_lock);
	}
	else
	{
	    pthread_cond_wait(&queue->m_cond, &queue->m_lock);
	}

    } while ( true );
}
