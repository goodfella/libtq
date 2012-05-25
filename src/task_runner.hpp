#ifndef TASK_RUNNER_HPP
#define TASK_RUNNER_HPP

#include <pthread.h>

namespace libtq
{
    class itask_queue;

    /// Wraps a thread that runs tasks
    class task_runner
    {
	public:

	task_runner();

	/** Starts running tasks from the given queue
	 *
	 *  @return Zero on success, non-zero on error
	 */
	int start(itask_queue* const queue);

	/// Joins with the task runner thread
	void join();

	private:

	bool m_started;
	pthread_t m_thread;

	/// Thread function that runs tasks
	static void* run_tasks(void* queue);
    };
}

#endif
