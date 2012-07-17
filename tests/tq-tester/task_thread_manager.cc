#include <unistd.h>

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
    m_desc(&m_task, queue, &m_stop_threads)
{}

void task_thread_manager::start_threads()
{
    m_sch_thread.start(&task_thread_manager::task_sch_handler, &m_desc, "task scheduler");
    m_scheduler_thread.start(&task_thread_manager::task_scheduler, &m_desc, "task scheduler");
    m_wait_thread.start(&task_thread_manager::wait_handler, &m_desc, "wait handler");
}

void task_thread_manager::stop_threads()
{
    // make sure the queue is started so the threads can clean up
    m_desc.queue->start_queue();

    // stop all the threads
    m_stop_threads.set(true);

    // make sure all the threads are stopped
    m_sch_thread.join();
    m_scheduler_thread.join();

    // Make sure the task is not scheduled
    m_desc.queue->flush();

    m_task.signal_waiters();

    m_wait_thread.join();
}

task_thread_manager::~task_thread_manager()
{
    stop_threads();
}

void task_thread_manager::print_stats() const
{
    cout << m_label << " run count = " << m_task.runcount() << endl
	 << m_label << " wait count = " << m_task.waitcount() << endl << endl;
}

void* task_thread_manager::task_sch_handler(void* t)
{
    task_thread_data* desc = static_cast<task_thread_data*>(t);

    while( desc->stop_thread->get() == false )
    {
	desc->taskp->schedule(desc->queue);
	pthread_yield();
    };

    pthread_exit(NULL);
}

void* task_thread_manager::task_scheduler(void* d)
{
    task_thread_data* data = static_cast<task_thread_data*>(d);

    static const int task_count = 10;
    test_task tasks[task_count];

    while ( data->stop_thread->get() == false )
    {
	for( int i = 0; i < task_count; ++i )
	{
	    tasks[i].schedule(data->queue);
	}

	for( int i = 0; i < task_count; ++i )
	{
	    tasks[i].wait();
	}

	pthread_yield();
    }

    data->queue->flush();

    return NULL;
}

void* task_thread_manager::wait_handler(void* d)
{
    task_thread_data* data = static_cast<task_thread_data*>(d);

    while( data->stop_thread->get() == false )
    {
	data->taskp->wait();
	pthread_yield();
    }

    return NULL;
}
