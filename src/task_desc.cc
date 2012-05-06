#include <signal.h>
#include <iostream>

#include "task_desc.hpp"
#include "itask.hpp"

using namespace std;
using namespace libtq;

wait_desc::wait_desc(pthread_mutex_t* m):
    mutex(m),
    next(this),
    prev(this),
    finished(false)
{
    pthread_cond_init(&condition, NULL);
}

wait_desc::wait_desc():
    mutex(NULL),
    next(this),
    prev(this),
    finished(true)
{}

wait_desc::wait_desc(const wait_desc& rhs):
    mutex(NULL),
    next(this),
    prev(this),
    finished(true)
{}

wait_desc::~wait_desc()
{
    remove_from_waitlist();

    if( mutex != NULL )
    {
	pthread_cond_destroy(&condition);
    }
}

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

int wait_desc::wait_for_task()
{
    sigset_t signal_mask, old_sigmask;
    int rc;

    // block all signals on this thread so that the thread can
    // properly clean up its resources
    rc = sigfillset(&signal_mask);

    if( rc != 0 )
    {
	return rc;
    }

    rc = pthread_sigmask(SIG_BLOCK, &signal_mask, &old_sigmask);

    if( rc != 0 )
    {
	return rc;
    }

    // we wait in a while loop because of spurious wakeups
    while( finished == false )
    {
	pthread_cond_wait(&condition, mutex);
    }

    // restore the signal mask
    rc = pthread_sigmask(SIG_BLOCK, &old_sigmask, NULL);

    return rc;
}

task_desc::task_desc(itask* task):
    m_task(task)
{}

void task_desc::run_task()
{
    m_task->run();
}

void task_desc::add_to_waitlist(wait_desc* desc)
{
    desc->add_to_waitlist(&m_waitlist);
}

void task_desc::signal_finished()
{
    // signal all the waiters that the task is finished
    wait_desc* desc;
    for(desc = m_waitlist.next; desc != &m_waitlist; desc = desc->next)
    {
	desc->finished = true;
	pthread_cond_signal(&desc->condition);
    }
}
