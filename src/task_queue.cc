#include <signal.h>
#include <algorithm>

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
	void cancel();

	private:

	pthread_mutex_t* m_lock;
    };
}

namespace
{
    struct shutdown_task : public itask
    {
	void run();
    };

    void shutdown_task::run()
    {
	pthread_cancel(pthread_self());
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

void mutex_lock::cancel()
{
    m_lock = NULL;
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

bool task_queue::start_queue()
{
    mutex_lock lock(&m_shutdown_lock);

    if( m_started == true )
    {
	return false;
    }

    int start_ret = pthread_create(&m_thread, NULL, task_runner, this);
    bool started = m_started = (start_ret == 0 ? true : false);

    return started;
}

void task_queue::stop_queue()
{
    pthread_mutex_lock(&m_shutdown_lock);
    pthread_mutex_lock(&m_lock);

    if( m_started == false )
    {
	pthread_mutex_unlock(&m_lock);
	pthread_mutex_unlock(&m_shutdown_lock);
	return;
    }

    shutdown_task kill_task_runner;

    try
    {
	priv_queue_task(&kill_task_runner);
    }
    catch(...)
    {
	pthread_mutex_unlock(&m_lock);
	pthread_mutex_unlock(&m_shutdown_lock);
	throw;
    }

    // at this point, kill_task_runner is in the back of the list
    priv_wait_for_task(m_tasks.back());

    // At this point, m_lock is unlocked and the task runner should be
    // waiting on the shutdown lock in it's cancelation routine
    while( m_started == true )
    {
	pthread_cond_wait(&m_shutdown_cond, &m_shutdown_lock);
    }

    pthread_mutex_unlock(&m_shutdown_lock);
}

void task_queue::priv_queue_task(itask * const task)
{
    // a task can only be scheduled once
    if( find(m_tasks.begin(), m_tasks.end(), task) == m_tasks.end() )
    {
	m_tasks.push_back(task_desc(task));

	// task was empty signal the task runner
	if( m_tasks.size() == 1 )
	{
	    pthread_cond_signal(&m_cond);
	}
    }
}

void task_queue::queue_task(itask * const task)
{
    mutex_lock lock(&m_lock);
    priv_queue_task(task);
}

int task_queue::cancel_task(itask * const task)
{
    int rc = 0;
    mutex_lock lock(&m_lock);

    if( m_tasks.front() == task )
    {
	// this condition means either the task runner has not
	// started processing this task, or has unlocked m_lock and is
	// about to run the task.  So we can't erase the task from the
	// list.

	// cancel the lock because wait_for_task will unlock it properly
	lock.cancel();

	task_desc td = m_tasks.front();
	rc = td.wait_for_task(&m_lock);
    }
    else
    {
	rc = 0;
	// This condition means that the task runner is not processing
	// this task, so we want to signal any waiters and erase the
	// task from the list.
	list<task_desc>::iterator canceled_task = find(m_tasks.begin(),
						       m_tasks.end(),
						       task);

	if( canceled_task != m_tasks.end() )
	{
	    canceled_task->signal_finished();
	    m_tasks.erase(canceled_task);
	}
    }
    
    return rc;
}

int task_queue::priv_wait_for_task(const task_desc& desc)
{
    task_desc my_desc = desc;
    return my_desc.wait_for_task(&m_lock);
}

int task_queue::priv_wait_for_task(itask * const task, mutex_lock& lock)
{
    int rc = 0;

    list<task_desc>::iterator req_task = find(m_tasks.begin(),
					      m_tasks.end(),
					      task);

    if( req_task != m_tasks.end() )
    {
	lock.cancel();

	// wait_for_task will properly unlock the lock
	rc = priv_wait_for_task(*req_task);
    }

    return rc;
}

int task_queue::wait_for_task(itask * const task)
{
    mutex_lock lock(&m_lock);
    return priv_wait_for_task(task, lock);
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

    pthread_cleanup_push(task_queue::handle_cancelation, queue);

    do
    {
	/* At this if statement, the lock is held from one of three
	 * places
	 *
	 * From the pthread_mutex_lock at the top of this function.
	 *
	 * From the pthread_mutex_lock at the end of the if statement.
	 *
	 * From the return of pthread_cond_wait in the else statement.
	 */
	if( queue->m_tasks.empty() == false )
	{
	    task_desc task = queue->m_tasks.front();

	    // set the cancel state to disable to prevent any tasks
	    // invoking the cancelation routines in an untimely manner
	    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

	    pthread_mutex_unlock(&queue->m_lock);

	    task.run_task();

	    pthread_mutex_lock(&queue->m_lock);

	    queue->m_tasks.pop_front();

	    // After the unlock above, there could be threads waiting
	    // for this task, so signal them.
	    task.signal_finished();

	    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	    // check if there are any pending cancelations
	    pthread_testcancel();

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

    pthread_cleanup_pop(1);
    pthread_exit(NULL);
}

void task_queue::handle_cancelation(void* tqueue)
{
    task_queue* queue = static_cast<task_queue*>(tqueue);

    pthread_mutex_unlock(&queue->m_lock);
    pthread_mutex_lock(&queue->m_shutdown_lock);

    queue->m_started = false;

    pthread_cond_broadcast(&queue->m_shutdown_cond);
    pthread_mutex_unlock(&queue->m_shutdown_lock);
}
