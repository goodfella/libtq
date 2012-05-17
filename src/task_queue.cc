#include <signal.h>
#include <algorithm>
#include <iostream>

#include "task_queue.hpp"
#include "itask.hpp"
#include "mutex_lock.hpp"
#include "task.hpp"
#include "task_cleanup.hpp"

using namespace std;
using namespace libtq;

namespace
{
    struct shutdown_exception {};

    struct shutdown_task : public itask
    {
	void run();
    };

    void shutdown_task::run()
    {
	throw shutdown_exception();
    }
}

task_queue::task_queue():
    m_started(false)
{
    pthread_mutex_init(&m_shutdown_lock, NULL);
}

task_queue::~task_queue()
{
    stop_queue();
    pthread_mutex_destroy(&m_shutdown_lock);
}

int task_queue::start_queue()
{
    mutex_lock lock(&m_shutdown_lock);

    if( m_started == true )
    {
	return 0;
    }

    int start_ret = pthread_create(&m_thread, NULL, task_runner, &m_queue);
    m_started = (start_ret == 0 ? true : false);

    return start_ret;
}

void task_queue::stop_queue()
{
    mutex_lock shutdown_lock(&m_shutdown_lock);

    if( m_started == false )
    {
	return;
    }

    shutdown_task kill_task_runner;

    // queue and wait for the shutdown task to finish
    queue_task(&kill_task_runner);
    wait_for_task(&kill_task_runner);

    pthread_join(m_thread, NULL);

    m_started = false;
}

void* task_queue::task_runner(void* tqueue)
{
    itask_queue * queue = static_cast<itask_queue*>(tqueue);

    do
    {
	try
	{
	    // run_task will block until a task is available
	    queue->run_task();
	}
	catch( const shutdown_exception& ex )
	{
	    return NULL;
	}

    } while ( true );

    return NULL;
}
