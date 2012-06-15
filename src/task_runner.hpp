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
	 */
	void start(itask_queue* const queue);

	/// Joins with the task runner thread
	void join();

	private:

	// No copying allowed since copying a pthread_t is not defined
	task_runner(const task_runner& rhs);
	task_runner& operator=(const task_runner& rhs);

	bool m_started;
	pthread_t m_thread;

	/// Thread function that runs tasks
	static void* run_tasks(void* queue);
    };
}

#endif
