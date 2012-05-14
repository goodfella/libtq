#include <signal.h>
#include <iostream>
#include <cstdio>

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

    m_next = this;
    m_prev = this;
}

void wait_desc::replace_head(wait_desc* new_head)
{
    if( m_next != this && m_prev != this )
    {
	// The list is not empty, replace its head
	new_head->m_next = m_next;
	new_head->m_prev = m_prev;

	m_prev->m_next = new_head;
	m_next->m_prev = new_head;

	// reset the old head so it can be destroyed properly
	m_next = this;
	m_prev = this;
    }
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

task_desc::task_desc():
    m_task(NULL)
{}

void task_desc::run_task()
{
    m_task->run();
}

void task_desc::add_to_waitlist(wait_desc* desc)
{
    desc->add_to_waitlist(&m_waitlist);
}

void task_desc::detach_listhead()
{
    m_waitlist.remove_from_waitlist();
}

void task_desc::move(task_desc& rhs)
{
    m_task = rhs.m_task;

    // uses this task_desc's m_waitlist as the head of the list
    rhs.m_waitlist.replace_head(&m_waitlist);
}

void task_desc::signal_finished()
{
    m_waitlist.signal_waiters();
}
