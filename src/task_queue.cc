#include <signal.h>
#include <algorithm>
#include <iostream>

#include "task_queue.hpp"
#include "task_desc.hpp"
#include "itask.hpp"

using namespace std;
using namespace libtq;

namespace libtq
{
    class mutex_lock
    {
	public:

	mutex_lock(pthread_mutex_t* lock);
	~mutex_lock();

	void unlock();
	void lock(pthread_mutex_t* lock);

	private:

	pthread_mutex_t* m_lock;
    };

    /// Runs tasks and handles cleanup in the event of an exception
    class task_handle
    {
	public:

	task_handle(task_queue * const queue);
	~task_handle();
	void run_task();

	private:

	task_queue* m_queue;
    };

    /// Cleans up the task runner after it's exited
    class task_runner_cleanup
    {
	public:

	task_runner_cleanup(task_queue* const queue);
	~task_runner_cleanup();

	private:

	task_queue* m_queue;
    };

    task_handle::task_handle(task_queue * const queue):
	m_queue(queue)
    {}

    task_handle::~task_handle()
    {
	// takes the queue lock, signals waiters and removes the task
	// from the queue
	pthread_mutex_lock(&m_queue->m_lock);
	m_queue->m_tasks.front().signal_finished();
	m_queue->m_tasks.pop_front();
    }
    
    void task_handle::run_task()
    {
	m_queue->m_tasks.front().run_task();
    }

    task_runner_cleanup::task_runner_cleanup(task_queue * const queue):
	m_queue(queue)
    {}
    
    task_runner_cleanup::~task_runner_cleanup()
    {
	// unlock the queue lock so waiters can wakeup
	pthread_mutex_unlock(&m_queue->m_lock);

	// take the shutdown lock to modify m_started
	pthread_mutex_lock(&m_queue->m_shutdown_lock);

	m_queue->m_started = false;

	// signal all queue waiters that the task runner exited
	pthread_cond_broadcast(&m_queue->m_shutdown_cond);
	pthread_mutex_unlock(&m_queue->m_shutdown_lock);
    }
}

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

mutex_lock::mutex_lock(pthread_mutex_t* lock):
    m_lock(lock)
{
    this->lock(lock);
}

mutex_lock::~mutex_lock()
{
    unlock();
}

void mutex_lock::unlock()
{
    if( m_lock != NULL )
    {
	pthread_mutex_unlock(m_lock);
	m_lock = NULL;
    }
}

void mutex_lock::lock(pthread_mutex_t* lock)
{
    if( lock != NULL )
    {
	pthread_mutex_lock(lock);
    }
}

task_queue::task_queue():
    m_started(false)
{
    pthread_mutex_init(&m_lock, NULL);
    pthread_mutex_init(&m_shutdown_lock, NULL);
    pthread_cond_init(&m_cond, NULL);
    pthread_cond_init(&m_shutdown_cond, NULL);
}

task_queue::~task_queue()
{
    stop_queue();
    pthread_cond_destroy(&m_shutdown_cond);
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_shutdown_lock);
    pthread_mutex_destroy(&m_lock);
}

int task_queue::start_queue()
{
    mutex_lock lock(&m_shutdown_lock);

    if( m_started == true )
    {
	return false;
    }

    int start_ret = pthread_create(&m_thread, NULL, task_runner, this);
    m_started = (start_ret == 0 ? true : false);

    if( m_started == true )
    {
	// detach the thread, so it's resources get cleaned up
	pthread_detach(m_thread);
    }

    return start_ret;
}

int task_queue::stop_queue()
{
    mutex_lock shutdown_lock(&m_shutdown_lock);

    if( m_started == false )
    {
	return 0;
    }

    shutdown_task kill_task_runner;
    if( queue_task_wait(&kill_task_runner) )
    {
	// there was an error waiting for the kill task, abort the
	// cleanup
	return 1;
    }

    // At this point, the task runner should be waiting on the
    // shutdown lock in its cancelation routine
    while( m_started == true )
    {
	pthread_cond_wait(&m_shutdown_cond, &m_shutdown_lock);
    }

    return 0;
}

bool task_queue::priv_queue_task(itask * const task)
{
    // a task can only be scheduled once
    if( find(m_tasks.begin(), m_tasks.end(), task) == m_tasks.end() )
    {
	m_tasks.push_back(task_desc(task));

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

int task_queue::queue_task_wait(itask * const task)
{
    int rc = 0;

    mutex_lock lock(&m_lock);

    // task was not allready queued
    if( priv_queue_task(task ) == true )
    {	
	rc = priv_wait_for_task(m_tasks.back());
	if( rc != 0 )
	{
	    // An error occured while trying to wait on the task, so
	    // remove the task from the queue and forward the error
	    m_tasks.pop_back();
	    return rc;
	}
    }

    return rc;
}

int task_queue::cancel_task(itask * const task, bool& cancel_status)
{
    int rc = 0;
    mutex_lock lock(&m_lock);

    if( m_tasks.front() == task )
    {
	// Being here means either the task runner has not started
	// processing this task, or has unlocked m_lock and is about
	// to run the task.  So we can't erase the task from the list.
	rc = priv_wait_for_task(m_tasks.front());

	cancel_status = false;
    }
    else
    {
	rc = 0;
	// Being here means that the task runner is not processing
	// this task, so we want to signal any waiters and erase the
	// task from the list.
	list<task_desc>::iterator canceled_task = find(m_tasks.begin(),
						       m_tasks.end(),
						       task);
	cancel_status = true;

	if( canceled_task != m_tasks.end() )
	{
	    canceled_task->signal_finished();
	    m_tasks.erase(canceled_task);
	}
    }
    
    return rc;
}

int task_queue::priv_wait_for_task(task_desc& td)
{
    wait_desc desc(&m_lock);
    td.add_to_waitlist(&desc);
    return desc.wait_for_task();
}

int task_queue::priv_wait_for_task(itask * const task)
{
    int rc = 0;

    list<task_desc>::iterator req_task = find(m_tasks.begin(),
					      m_tasks.end(),
					      task);

    if( req_task != m_tasks.end() )
    {
	rc = priv_wait_for_task(*req_task);
    }

    return rc;
}

int task_queue::wait_for_task(itask * const task)
{
    mutex_lock lock(&m_lock);
    return priv_wait_for_task(task);
}

void* task_queue::task_runner(void* tqueue)
{
    sigset_t signal_mask;
    int rc = 0;

    task_queue * queue = static_cast<task_queue*>(tqueue);

    // block all signals on this thread so that the thread can
    // properly clean up its resources

    rc = sigfillset(&signal_mask);
    if( rc != 0 )
    {
	pthread_exit(NULL);
    }

    rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
    
    if( rc != 0 )
    {
	pthread_exit(NULL);
    }

    pthread_mutex_lock(&queue->m_lock);

    // during stack unwinding, this object will cleanup the task
    // runner thread
    task_runner_cleanup cleanup(queue);

    do
    {
	/* At this if statement, the lock is held from one of three
	 * places
	 *
	 * From the pthread_mutex_lock at the top of this function.
	 *
	 * From the pthread_mutex_lock inside the task_handle destructor
	 *
	 * From the return of pthread_cond_wait in the else statement.
	 */
	if( queue->m_tasks.empty() == false )
	{
	    pthread_mutex_unlock(&queue->m_lock);

	    try
	    {
		task_handle task(queue);
		task.run_task();
	    }
	    catch (const shutdown_exception& ex)
	    {
		// we need to shutdown
		pthread_exit(NULL);
	    }

	    // give the waiting threads a chance to run
	    pthread_mutex_unlock(&queue->m_lock);
	    pthread_yield();
	    pthread_mutex_lock(&queue->m_lock);
	}
	else
	{
	    pthread_cond_wait(&queue->m_cond, &queue->m_lock);
	}

    } while ( true );
}
