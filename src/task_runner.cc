#include <stdexcept>
#include "task_runner.hpp"
#include "itask_queue.hpp"
#include "runner_canceled.hpp"

using namespace libtq;

task_runner::task_runner():
    m_started(false)
{}

void task_runner::start(itask_queue* const queue)
{
    if( m_started == true )
    {
	return;
    }

    if( pthread_create(&m_thread, NULL, &task_runner::run_tasks, queue) != 0 )
    {
	throw std::runtime_error("failed to start task_runner thread");
    }

    m_started = true;
}

void task_runner::join()
{
    if( m_started == false )
    {
	return;
    }

    pthread_join(m_thread, NULL);

    m_started = false;
}

void* task_runner::run_tasks(void* q)
{
    itask_queue * queue = static_cast<itask_queue*>(q);

    do
    {
	try
	{
	    // run_task will block until a task is available
	    queue->run_task();
	}
	catch (const runner_canceled& ex )
	{
	    // task runner was canceled
	    return NULL;
	}

    } while ( true );

    return NULL;
}
