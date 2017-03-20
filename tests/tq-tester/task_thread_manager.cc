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
    m_sch_thread = std::thread{&task_thread_manager::task_sch_handler, &m_desc};
    m_scheduler_thread = std::thread{&task_thread_manager::task_scheduler, &m_desc};
    m_wait_thread = std::thread{&task_thread_manager::wait_handler, &m_desc};
}

void task_thread_manager::stop_threads()
{
    if (m_stop_threads.get())
    {
        return;
    }

    // make sure the queue is started so the threads can clean up
    m_desc.queue->start_queue();

    // stop all the threads
    m_stop_threads.set(true);

    // make sure all the threads are stopped
    if ( m_sch_thread.joinable() )
    {
        m_sch_thread.join();
    }

    if ( m_scheduler_thread.joinable() )
    {
        m_scheduler_thread.join();
    }

    // Make sure the task is not scheduled
    m_desc.queue->flush();

    if ( m_wait_thread.joinable() )
    {
        m_wait_thread.join();
    }
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

void task_thread_manager::task_sch_handler(task_thread_data* data)
{
    while( data->stop_thread->get() == false )
    {
        data->queue->queue_task(data->taskp);
        std::this_thread::yield();
    };
}

void task_thread_manager::task_scheduler(task_thread_data* data)
{
    static const int task_count = 10;
    test_task tasks[task_count];

    while ( data->stop_thread->get() == false )
    {
	for( int i = 0; i < task_count; ++i )
	{
	    data->queue->queue_task(&tasks[i]);
	}

	for( int i = 0; i < task_count; ++i )
	{
	    tasks[i].wait();
	}

        std::this_thread::yield();
    }

    data->queue->flush();
}

void task_thread_manager::wait_handler(task_thread_data* data)
{
    while( data->stop_thread->get() == false )
    {
	data->taskp->wait();
        std::this_thread::yield();
    }
}
