#include <algorithm>
#include <functional>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "itask_queue.hpp"
#include "itask.hpp"
#include "runner_canceled.hpp"

using namespace libtq;

itask_queue::itask_queue()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cancel = false;
}

void itask_queue::queue_task(itask * const itaskp)
{
    bool signal = false;

    {
	/* There's a separate scope here so the signal can be done
	 * without holding the mutex. */

	std::lock_guard<std::mutex> lock(m_mutex);

	if( std::find(m_tasks.begin(), m_tasks.end(), itaskp) == m_tasks.end() )
	{
	    m_tasks.push_back(itaskp);

	    try
	    {
		itaskp->scheduled();
	    }
	    catch(...)
	    {
		/* If an exception is thrown in itaskp->scheduled()
		 * then remove the itask from the queue.  This insures
		 * the strong exception guarantee provided by
		 * queue_task. */
		m_tasks.pop_back();
		throw;
	    }

	    // task queue was empty signal the task runner
	    if( m_tasks.size() == 1 )
	    {
		signal = true;
	    }
	}
    }

    if( signal == true )
    {
	/* Signal a waiting thread if the queue was previously empty */
	m_cond.notify_one();
    }
}

void itask_queue::set_cancel()
{
    {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_cancel = true;
    }

    // Wake up any waiting task runners
    m_cond.notify_all();
}

void itask_queue::clear_cancel()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cancel = false;
}

void itask_queue::run_task()
{
    itask* task = NULL;

    {
	std::unique_lock<std::mutex> lock(m_mutex);

	if( m_cancel == true )
	{
	    throw runner_canceled();
	}

	m_cond.wait(lock,
		    [&] {
			    if (m_cancel == true) {
				return true;
			    }

			    return m_tasks.empty() ? false : true;
		    });

	if (m_cancel == true)
	{
	    throw runner_canceled();
	}

	// At this point the queue is not empty, and we have the lock
	// from either the std::unique_lock at the top, or from the
	// condition wait

	// Get a copy of the task pointer and remove the task from the
	// task list

	task = m_tasks.front();
	m_tasks.pop_front();
    }

    task->run();
}
