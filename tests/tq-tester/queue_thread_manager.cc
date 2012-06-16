#include <unistd.h>
#include <stdexcept>
#include <iostream>

#include "queue_thread_manager.hpp"
#include "task_queue.hpp"

using namespace std;
using namespace libtq;
using namespace tq_tester;

queue_thread_data::queue_thread_data(libtq::task_queue* const q, bool_flag* const f):
    queue(q),
    stop_thread(f)
{}

queue_thread_manager::queue_thread_manager(task_queue* const queue):
    m_data(queue, &m_stop_threads)
{}

queue_thread_manager::~queue_thread_manager()
{
    stop_threads();
}

void queue_thread_manager::start_threads()
{
    m_shutdown_thread.start(queue_thread_manager::shutdown_queue, &m_data, "shutdown queue thread");
    m_cancel_tasks_thread.start(queue_thread_manager::cancel_tasks, &m_data, "cancel tasks thread");
    m_stop_thread.start(queue_thread_manager::stop_queue, &m_data, "stop queue thread");
    m_cancel_queue_thread.start(queue_thread_manager::cancel_queue, &m_data, "cancel queue thread");
    m_flush_thread.start(queue_thread_manager::flush_queue, &m_data, "flush queue thread");
}

void queue_thread_manager::stop_threads()
{
    m_stop_threads.set(true);

    m_shutdown_thread.join();
    m_cancel_tasks_thread.join();
    m_stop_thread.join();
    m_cancel_queue_thread.join();
    m_flush_thread.join();
}

void* queue_thread_manager::shutdown_queue(void* d)
{
    queue_thread_data* data = static_cast<queue_thread_data*>(d);

    while ( data->stop_thread->get() == false )
    {
	data->queue->start_queue();
	sleep(1);
	data->queue->shutdown_queue();
    }

    return NULL;
}

void* queue_thread_manager::cancel_tasks(void* d)
{
    queue_thread_data* data = static_cast<queue_thread_data*>(d);

    while( data->stop_thread->get() == false )
    {
	data->queue->cancel_tasks();
	sleep(1);
    }

    return NULL;
}

void* queue_thread_manager::stop_queue(void* d)
{
    queue_thread_data* data = static_cast<queue_thread_data*>(d);

    while( data->stop_thread->get() == false )
    {
	data->queue->stop_queue();
	sleep(1);
    }

    return NULL;
}

void* queue_thread_manager::cancel_queue(void* d)
{
    queue_thread_data* data = static_cast<queue_thread_data*>(d);

    while( data->stop_thread->get() == false )
    {
	data->queue->cancel_queue();
	sleep(1);
    }

    return NULL;
}

void* queue_thread_manager::flush_queue(void* d)
{
    queue_thread_data* data = static_cast<queue_thread_data*>(d);

    while( data->stop_thread->get() == false )
    {
	data->queue->flush();
    }

    return NULL;
}
