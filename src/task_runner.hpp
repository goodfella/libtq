#ifndef TASK_RUNNER_HPP
#define TASK_RUNNER_HPP

#include <thread>

namespace libtq
{
    class itask_queue;

    /// Wraps a thread that runs tasks
    class task_runner
    {
	public:

	task_runner();

	task_runner(const task_runner&) = delete;
	task_runner& operator=(const task_runner&) = delete;

	/** Starts running tasks from the given queue
	 *
	 */
	void start(itask_queue* const queue);

	/// Joins with the task runner thread
	void join();

	private:

	bool m_started;
	std::thread m_thread;

	/// Thread function that runs tasks
	static void run_tasks(itask_queue* queue);
    };
}

#endif
