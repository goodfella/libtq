#include "task.hpp"
#include "itask.hpp"
#include "mutex_lock.hpp"
#include "rwlock_rdlock.hpp"
#include "rwlock_wrlock.hpp"

using namespace std;
using namespace libtq;

task::task():
    m_task(NULL),
    m_finished(false),
    m_canceled(false),
    m_refcount(0)
{
    pthread_mutex_init(&m_state_lock, NULL);
    pthread_rwlock_init(&m_task_lock, NULL);
    pthread_mutex_init(&m_ref_lock, NULL);
    pthread_cond_init(&m_cond, NULL);
}

task::~task()
{
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_ref_lock);
    pthread_rwlock_destroy(&m_task_lock);
    pthread_mutex_destroy(&m_state_lock);
}

void task::run_task() const
{
    itask* itaskp = NULL;

    {
	/* The lock here is only used to execute the necessary memory
	 * barriers.  The m_task member will not be changed after the
	 * lock is released, because it's only set when the task is
	 * freed.  The task_handle class only frees a task object when
	 * the task object's ref count is zero. */
	rwlock_rdlock lock(&m_task_lock);
	itaskp = m_task;
    }

    itaskp->run();
}

void task::signal_finished()
{
    {
	/* No need to hold the lock when signaling the waiters */
	mutex_lock lock(&m_state_lock);
	m_finished = true;
    }

    pthread_cond_broadcast(&m_cond);
}

void task::signal_canceled()
{
    {
	mutex_lock lock(&m_state_lock);
	m_canceled = true;
    }

    pthread_cond_broadcast(&m_cond);
}

const bool task::wait_for_task()
{
    // boolean indicating whether or not the task was ran
    bool task_ran = false;

    mutex_lock lock(&m_state_lock);

    // wait for the task to finish
    while( m_finished == false && m_canceled == false )
    {
	pthread_cond_wait(&m_cond, &m_state_lock);
    }

    task_ran = m_finished;
    return task_ran;
}

void task::reset(task* const next)
{
    {
	mutex_lock lock(&m_state_lock);
	m_finished = false;
	m_canceled = false;
    }

    {
	rwlock_wrlock lock(&m_task_lock);
	m_task = NULL;
	m_next = next;
    }
}

void task::get_ref()
{
    mutex_lock lock(&m_ref_lock);
    ++m_refcount;
}

int task::put_ref()
{
    int new_count;
    mutex_lock lock(&m_ref_lock);

    --m_refcount;
    new_count = m_refcount;

    return new_count;
}

const bool task::operator==(itask const * const taskp) const
{
    rwlock_rdlock lock(&m_task_lock);
    return taskp == m_task;
}

task* const task::itaskp(itask * const taskp)
{
    rwlock_wrlock lock(&m_task_lock);
    task* next = m_next;
    m_task = taskp;

    return next;
}
