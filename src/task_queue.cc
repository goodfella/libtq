#include <signal.h>
#include <algorithm>
#include <iostream>

#include "task_queue.hpp"
#include "itask.hpp"
#include "mutex_lock.hpp"
#include "shutdown_task.hpp"
#include "wait_task.hpp"

using namespace std;
using namespace libtq;

task_queue::task_queue()
{
    pthread_mutex_init(&m_shutdown_lock, NULL);

    mutex_lock lock(&m_shutdown_lock);

    m_started = false;
}

task_queue::~task_queue()
{
    stop_queue();
    pthread_mutex_destroy(&m_shutdown_lock);
}

void task_queue::cancel_tasks()
{
    /* The shutdown lock is taken here to prevent any shutdown tasks
     * from being canceled */
    mutex_lock lock(&m_shutdown_lock);

    m_queue.cancel_tasks();
}

inline void task_queue::locked_start_queue()
{
    m_task_runner.start(&m_queue);
    m_started = true;
}

void task_queue::start_queue()
{
    mutex_lock lock(&m_shutdown_lock);

    if( m_started == true )
    {
	return;
    }

    locked_start_queue();
}

void task_queue::shutdown_queue()
{
    mutex_lock shutdown_lock(&m_shutdown_lock);

    if( m_started == false )
    {
	return;
    }

    shutdown_task kill_task_runner;

    // queue and wait for the shutdown task to finish
    queue_task(&kill_task_runner);
    kill_task_runner.wait_for_completion();

    m_task_runner.join();

    m_started = false;
}

void task_queue::flush()
{
    mutex_lock lock(&m_shutdown_lock);

    bool stop_queue = false;

    m_queue.queue_task(&m_wait_task);

    if( m_started == false )
    {
	// We have to start the queue because at this point any thread
	// that calls start_queue will block due to flush having the
	// m_shutdown_lock.

	try
	{
	    locked_start_queue();
	}
	catch(std::exception& ex)
	{
	    // Error starting the queue, so cancel the wait task and
	    // rethrow the exception.  The wait task is canceled so
	    // that flush has the strong exception guarantee.
	    m_queue.cancel_task(&m_wait_task);
	    throw;
	}

	stop_queue = true;
    }

    m_wait_task.wait();

    if( stop_queue == true )
    {
	locked_stop_queue();
    }
}

void task_queue::locked_stop_queue()
{
    // Set the cancel flag in the queue and wait until the task runner
    // exits
    m_queue.set_cancel();
    m_task_runner.join();
    m_queue.clear_cancel();

    m_started = false;
}

void task_queue::stop_queue()
{
    mutex_lock shutdown_lock(&m_shutdown_lock);

    if( m_started == false )
    {
	return;
    }

    locked_stop_queue();
}

void task_queue::cancel_queue()
{
    mutex_lock shutdown_lock(&m_shutdown_lock);

    m_queue.cancel_tasks();
    locked_stop_queue();
}
