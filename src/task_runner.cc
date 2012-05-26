#include "task_runner.hpp"
#include "itask_queue.hpp"
#include "shutdown_task.hpp"

using namespace libtq;

task_runner::task_runner():
    m_started(false)
{}

int task_runner::start(itask_queue* const queue)
{
    if( m_started == true )
    {
	return 0;
    }

    int ret = pthread_create(&m_thread, NULL, &task_runner::run_tasks, queue);
    m_started = ret == 0 ? true : false;

    return ret;
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
	catch( const shutdown_exception& ex )
	{
	    // Time to shutdown
	    return NULL;
	}

    } while ( true );

    return NULL;
}
