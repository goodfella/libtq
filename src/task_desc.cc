#include <signal.h>
#include <iostream>

#include "task_desc.hpp"
#include "itask.hpp"

using namespace std;
using namespace libtq;

wait_desc::wait_desc():
    m_next(this),
    m_prev(this),
    m_finished(false),
    m_condition(NULL)
{}

wait_desc::wait_desc(const wait_desc& rhs):
    m_next(this),
    m_prev(this),
    m_finished(false),
    m_condition(NULL)
{}

wait_desc::~wait_desc()
{
    remove_from_waitlist();
}

void wait_desc::add_to_waitlist(wait_desc* wait_list)
{
    wait_list->m_next->m_prev = this;
    m_next = wait_list->m_next;
    m_prev = wait_list;
    wait_list->m_next = this;
}

void wait_desc::remove_from_waitlist()
{
    m_next->m_prev = m_prev;
    m_prev->m_next = m_next;
}

void wait_desc::wait_for_task(pthread_mutex_t* mutex)
{
    pthread_cond_t condition;
    pthread_cond_init(&condition, NULL);
    m_condition = &condition;

    // we wait in a while loop because of spurious wakeups
    while( m_finished == false )
    {
	pthread_cond_wait(&condition, mutex);
    }

    pthread_cond_destroy(&condition);
}

void wait_desc::signal_waiters()
{
    // signal all the waiters that the task is finished

    wait_desc* desc;
    for(desc = m_next; desc != this; desc = desc->m_next)
    {
	desc->m_finished = true;
	pthread_cond_signal(desc->m_condition);
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
