#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <thread>
#include <chrono>

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

void queue_thread_manager::start_threads()
{
    m_shutdown_thread = std::thread{&queue_thread_manager::shutdown_queue, &m_data};
    m_stop_thread = std::thread{&queue_thread_manager::stop_queue, &m_data};
    m_flush_thread = std::thread{&queue_thread_manager::flush_queue, &m_data};
}

void queue_thread_manager::stop_threads()
{
    m_stop_threads.set(true);

    if ( m_shutdown_thread.joinable())
    {
        m_shutdown_thread.join();
    }

    if ( m_stop_thread.joinable() )
    {
        m_stop_thread.join();
    }

    if ( m_flush_thread.joinable() )
    {
        m_flush_thread.join();
    }
}

void queue_thread_manager::shutdown_queue(queue_thread_data* data)
{
    while ( data->stop_thread->get() == false )
    {
	data->queue->start_queue();
        std::this_thread::sleep_for(std::chrono::seconds(1));
	data->queue->shutdown_queue();
    }
}

void queue_thread_manager::stop_queue(queue_thread_data* data)
{
    while( data->stop_thread->get() == false )
    {
	data->queue->stop_queue();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void queue_thread_manager::flush_queue(queue_thread_data* data)
{
    while( data->stop_thread->get() == false )
    {
	data->queue->flush();
        std::this_thread::yield();
    }
}
