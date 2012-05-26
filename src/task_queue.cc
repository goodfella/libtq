#include <signal.h>
#include <algorithm>
#include <iostream>

#include "task_queue.hpp"
#include "itask.hpp"
#include "mutex_lock.hpp"
#include "task.hpp"
#include "task_cleanup.hpp"
#include "shutdown_task.hpp"

using namespace std;
using namespace libtq;

task_queue::task_queue():
    m_started(false)
{
    pthread_mutex_init(&m_shutdown_lock, NULL);
}

task_queue::~task_queue()
{
    shutdown_queue();
    pthread_mutex_destroy(&m_shutdown_lock);
}

void task_queue::cancel_tasks()
{
    /* The shutdown lock is taken here to prevent any shutdown tasks
     * from being canceled */
    mutex_lock lock(&m_shutdown_lock);

    m_queue.cancel_tasks();
}

int task_queue::start_queue()
{
    mutex_lock lock(&m_shutdown_lock);

    if( m_started == true )
    {
	return 0;
    }

    int start_ret = m_task_runner.start(&m_queue);
    m_started = (start_ret == 0 ? true : false);

    return start_ret;
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
    wait_for_task(&kill_task_runner);

    m_task_runner.join();

    m_started = false;
}

void task_queue::locked_stop_queue()
{
    if( m_started == false )
    {
	return;
    }

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
    locked_stop_queue();
}

void task_queue::cancel_queue()
{
    mutex_lock shutdown_lock(&m_shutdown_lock);

    m_queue.cancel_tasks();
    locked_stop_queue();
}
