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

    m_thread = std::thread{&task_runner::run_tasks, queue};
    m_started = true;
}

void task_runner::join()
{
    if( m_started == false )
    {
	return;
    }

    m_thread.join();
    m_started = false;
}

void task_runner::run_tasks(itask_queue* queue)
{
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
	    return;
	}

    } while ( true );
}
