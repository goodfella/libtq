#include <stdexcept>
#include <iostream>

#include "task_thread_manager.hpp"
#include "task_queue.hpp"

using namespace std;
using namespace tq_tester;
using namespace libtq;

task_thread_data::task_thread_data(test_task* t, task_queue* const q, bool_flag* f):
    taskp(t),
    queue(q),
    stop_thread(f)
{}

task_thread_manager::task_thread_manager(const string& label, task_queue * const queue):
    m_label(label),
    m_desc(&m_task, queue, &m_stop_threads),
    m_sch_started(false),
    m_sch_wait_started(false),
    m_cancel_started(false),
    m_wait_started(false)
{}

void task_thread_manager::start_threads()
{
    if( pthread_create(&m_task_sch, NULL, task_thread_manager::task_sch_handler, &m_desc) != 0 )
    {
	throw std::runtime_error("error creating task scheduler thread");
    }

    m_sch_started = true;

    if( pthread_create(&m_task_sch_wait, NULL, task_thread_manager::task_sch_wait_handler, &m_desc) != 0 )
    {
	throw std::runtime_error("error creating task schedule with a wait thread");
    }

    m_sch_wait_started = true;

    if( pthread_create(&m_task_cancel, NULL, task_thread_manager::task_cancel_handler, &m_desc) != 0 )
    {
	throw std::runtime_error("error creating task canceler thread");
    }

    m_cancel_started = true;

    if( pthread_create(&m_task_wait, NULL, task_thread_manager::task_wait_handler, &m_desc) != 0 )
    {
	throw std::runtime_error("error creating task wait thread");
    }

    m_wait_started = true;
}

void task_thread_manager::stop_threads()
{
    if( m_sch_started == false &&
	m_sch_wait_started == false &&
	m_wait_started == false &&
	m_cancel_started == false )
    {
	return;
    }

    // make sure the queue is started so the threads can clean up
    m_desc.queue->start_queue();

    // stop all the threads
    m_stop_threads.set(true);

    if( m_sch_started == true )
    {
	pthread_join(m_task_sch, NULL);
	m_sch_started = false;
    }

    if( m_sch_wait_started == true )
    {
	pthread_join(m_task_sch_wait, NULL);
	m_sch_wait_started = false;
    }

    if( m_wait_started == true )
    {
	pthread_join(m_task_wait, NULL);
	m_wait_started = false;
    }

    if( m_cancel_started == true )
    {
	pthread_join(m_task_cancel, NULL);
	m_cancel_started = false;
    }

    // wait for the task to finish
    m_desc.queue->wait_for_task(&m_task);
}

task_thread_manager::~task_thread_manager()
{
    stop_threads();
}

void task_thread_manager::print_stats() const
{
    cout << m_label << " run count = " << m_task.runcount() << endl
	 << m_label << " cancel count = " << m_task.cancelcount() << endl << endl;
}

void* task_thread_manager::task_sch_handler(void* t)
{
    task_thread_data* desc = static_cast<task_thread_data*>(t);

    while( desc->stop_thread->get() == false )
    {
	desc->queue->queue_task(desc->taskp);
	pthread_yield();
    };

    pthread_exit(NULL);
}

void* task_thread_manager::task_sch_wait_handler(void* t)
{
    task_thread_data* desc = static_cast<task_thread_data*>(t);

    while( desc->stop_thread->get() == false )
    {
	desc->queue->queue_task(desc->taskp);
	desc->queue->wait_for_task(desc->taskp);
	pthread_yield();
    };

    pthread_exit(NULL);
}

void* task_thread_manager::task_cancel_handler(void* t)
{
    task_thread_data* desc = static_cast<task_thread_data*>(t);

    while( desc->stop_thread->get() == false )
    {
	if( desc->queue->cancel_task(desc->taskp) == true )
	{
	    desc->taskp->inc_cancelcount();
	}

	pthread_yield();
    }

    pthread_exit(NULL);
}

void* task_thread_manager::task_wait_handler(void* t)
{
    task_thread_data* desc = static_cast<task_thread_data*>(t);

    while( desc->stop_thread->get() == false )
    {
	desc->queue->wait_for_task(desc->taskp);
	pthread_yield();
    };

    pthread_exit(NULL);
}
