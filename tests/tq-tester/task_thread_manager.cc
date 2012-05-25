#include <stdexcept>
#include <iostream>
#include <vector>

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
    m_desc(&m_task, queue, &m_stop_threads)
{}

void task_thread_manager::start_threads()
{
    m_sch_thread.start(&task_thread_manager::task_sch_handler, &m_desc, "task scheduler");
    m_sch_wait_thread.start(&task_thread_manager::task_sch_wait_handler, &m_desc, "task wait scheduler");
    m_cancel_thread.start(&task_thread_manager::task_cancel_handler, &m_desc, "task canceler");
    m_wait_thread.start(&task_thread_manager::task_wait_handler, &m_desc, "task waiter");
    m_scheduler_thread.start(&task_thread_manager::task_scheduler, &m_desc, "task schedular");
}

void task_thread_manager::stop_threads()
{
    // make sure the queue is started so the threads can clean up
    m_desc.queue->start_queue();

    // stop all the threads
    m_stop_threads.set(true);

    // make sure all the threads are stopped
    m_sch_thread.join();
    m_sch_wait_thread.join();
    m_cancel_thread.join();
    m_wait_thread.join();
    m_scheduler_thread.join();

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
	 << m_label << " cancel count = " << m_task.cancelcount() << endl
	 << m_label << " wait count = " << m_task.waitcount() << endl << endl;
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

	if( desc->queue->wait_for_task(desc->taskp) > 0 )
	{
	    desc->taskp->inc_waitcount();
	}

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
	if( desc->queue->wait_for_task(desc->taskp) > 0 )
	{
	    desc->taskp->inc_waitcount();
	}

	pthread_yield();
    };

    pthread_exit(NULL);
}

void* task_thread_manager::task_scheduler(void* d)
{
    task_thread_data* data = static_cast<task_thread_data*>(d);

    vector<test_task> tasks(10);

    while ( data->stop_thread->get() == false )
    {
	for(vector<test_task>::iterator i = tasks.begin();
	    i != tasks.end();
	    ++i)
	{
	    data->queue->queue_task(&(*i));
	}

	for(vector<test_task>::iterator i = tasks.begin();
	    i != tasks.end();
	    ++i)
	{
	    data->queue->wait_for_task(&(*i));
	}
    }

    return NULL;
}
