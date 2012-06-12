#include <unistd.h>

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
    m_cancel_thread.start(&task_thread_manager::task_cancel_handler, &m_desc, "task canceler");
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
    m_cancel_thread.join();
    m_scheduler_thread.join();

    // cancel the task if it's still queued
    if( m_desc.queue->cancel_task(&m_task) == false )
    {
	m_desc.queue->wait_for_tasks();
    }
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

void* task_thread_manager::task_cancel_handler(void* t)
{
    task_thread_data* desc = static_cast<task_thread_data*>(t);

    while( desc->stop_thread->get() == false )
    {
	desc->queue->cancel_task(desc->taskp);
	sleep(1);
    }

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
    }

    for(vector<test_task>::iterator i = tasks.begin();
	i != tasks.end();
	++i)
    {
	data->queue->cancel_task(&(*i));
    }

    data->queue->wait_for_tasks();

    return NULL;
}
