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
    m_stop_start_thread.start(queue_thread_manager::stop_start_queue, &m_data, "stop queue thread");
    m_cancel_tasks_thread.start(queue_thread_manager::cancel_tasks, &m_data, "cancel tasks thread");
}

void queue_thread_manager::stop_threads()
{
    m_stop_threads.set(true);

    m_stop_start_thread.join();
    m_cancel_tasks_thread.join();
}

void* queue_thread_manager::stop_start_queue(void* d)
{
    queue_thread_data* data = static_cast<queue_thread_data*>(d);

    while ( data->stop_thread->get() == false )
    {
	if( data->queue->start_queue() )
	{
	    cerr << "error starting task queue\n";
	    break;
	}

	pthread_yield();

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
	pthread_yield();
    }

    return NULL;
}
