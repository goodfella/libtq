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

void wait_desc::wait_for_task()
{
    // we wait in a while loop because of spurious wakeups
    while( finished == false )
    {
	pthread_cond_wait(&condition, mutex);
    }
}

void wait_desc::signal_waiters()
{
    // signal all the waiters that the task is finished

    wait_desc* desc;
    for(desc = next; desc != this; desc = desc->next)
    {
	desc->finished = true;
	pthread_cond_signal(&desc->condition);
    }
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
    m_waitlist.signal_waiters();
}
