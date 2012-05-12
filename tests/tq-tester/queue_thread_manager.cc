#include <stdexcept>
#include <iostream>

#include "queue_thread_manager.hpp"
#include "task_queue.hpp"

using namespace std;
using namespace libtq;
using namespace tq_tester;

queue_thread_manager::queue_thread_manager(task_queue* const queue):
    m_queue(queue),
    m_thread_started(false)
{}

queue_thread_manager::~queue_thread_manager()
{
    stop_thread();
}

void queue_thread_manager::start_thread()
{
    if( pthread_create(&m_thread, NULL, &queue_thread_manager::queue_thread, m_queue) != 0 )
    {
	throw std::runtime_error("error running queue thread");
    }

    m_thread_started = true;
}

void queue_thread_manager::stop_thread()
{
    if( m_thread_started == false )
    {
	return;
    }

    m_stop_thread.set(true);

    if( m_thread_started == true )
    {
	pthread_join(m_thread, NULL);
	m_thread_started = false;
    }
}

bool_flag queue_thread_manager::m_stop_thread;

void* queue_thread_manager::queue_thread(void* q)
{
    task_queue * queue = static_cast<task_queue*>(q);

    while ( m_stop_thread.get() == false )
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
