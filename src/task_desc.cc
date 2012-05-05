#include <signal.h>

#include "task_desc.hpp"
#include "itask.hpp"

using namespace libtq;

wait_desc::wait_desc(pthread_mutex_t* m, pthread_cond_t* cond, bool* fini):
    mutex(m),
    condition(cond),
    finished(fini),
    next(this),
    prev(this)
{}

void wait_desc::add_to_waitlist(wait_desc* wait_list)
{
    wait_list->next->prev = this;
    next = wait_list->next;
    prev = wait_list;
    wait_list->next = this;
}

void wait_desc::remove_from_waitlist()
{
    next->prev = prev;
    prev->next = next;
}

task_desc::task_desc(itask* task):
    m_task(task),
    m_waitlist(NULL)
{}

void task_desc::run_task()
{
    // ignore all exceptions
    try
    {
	m_task->run();
    }
    catch (...)
    {}
}

void task_desc::add_to_waitlist(wait_desc* desc)
{
    if( m_waitlist == NULL )
    {
	m_waitlist = desc;
    }
    else
    {
	desc->add_to_waitlist(m_waitlist);
    }
}

void task_desc::cleanup_waiter(void* waiter)
{
    wait_desc* desc = static_cast<wait_desc*>(waiter);
    pthread_mutex_unlock(desc->mutex);
    pthread_cond_destroy(desc->condition);

    desc->remove_from_waitlist();
}

int task_desc::wait_for_task(pthread_mutex_t* lock)
{
    bool finished = false;
    pthread_cond_t cond;
    sigset_t signal_mask, old_sigmask;
    int rc;

    // block all signals on this thread so that the thread can
    // properly clean up its resources
    rc = sigfillset(&signal_mask);

    if( rc != 0 )
    {
	pthread_mutex_unlock(lock);
	return rc;
    }

    rc = pthread_sigmask(SIG_BLOCK, &signal_mask, &old_sigmask);

    if( rc != 0 )
    {
	pthread_mutex_unlock(lock);
	return rc;
    }

    pthread_cond_init(&cond, NULL);
    wait_desc desc(lock, &cond, &finished);
    add_to_waitlist(&desc);

    // insures that if the thread is canceled, the wait_desc is pulled
    // off the waiters list and the mutex is unlocked
    pthread_cleanup_push(task_desc::cleanup_waiter, &desc);

    // we wait in a while loop because of spurious wakeups
    while( finished == false )
    {
	pthread_cond_wait(&cond, lock);
    }

    // cleanup_waiter removes the wait_desc from the waiter list
    pthread_cleanup_pop(1);

    // restore the signal mask
    rc = pthread_sigmask(SIG_BLOCK, &old_sigmask, NULL);

    return rc;
}

void task_desc::signal_finished()
{
    if( m_waitlist == NULL )
    {
	return;
    }

    // signal all the waiters that the task is finished
    wait_desc* desc;
    for(desc = m_waitlist->next; desc != m_waitlist; desc = desc->next)
    {
	*desc->finished = true;
	pthread_cond_signal(desc->condition);
    }
}
