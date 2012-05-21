#include <stdexcept>
#include <iostream>

#include "queue_thread_manager.hpp"
#include "task_queue.hpp"

using namespace std;
using namespace libtq;
using namespace tq_tester;

bool_flag queue_thread_manager::m_stop_threads;

queue_thread_manager::queue_thread_manager(task_queue* const queue):
    m_queue(queue)
{}

queue_thread_manager::~queue_thread_manager()
{
    stop_thread();
}

void queue_thread_manager::start_thread()
{
    m_stop_thread.start(queue_thread_manager::queue_thread, m_queue, "stop queue thread");
}

void queue_thread_manager::stop_thread()
{
    m_stop_threads.set(true);

    m_stop_thread.join();
}

void* queue_thread_manager::queue_thread(void* q)
{
    task_queue * queue = static_cast<task_queue*>(q);

    while ( m_stop_threads.get() == false )
    {
	if( queue->start_queue() )
	{
	    cerr << "error starting task queue\n";
	    break;
	}

	pthread_yield();

	queue->stop_queue();
    }

    pthread_exit(NULL);
}
