#ifndef TASK_MANAGER_HPP
#define TASK_MANAGER_HPP

#include <string>
#include <vector>

namespace libtq
{
    class task_queue;
}

namespace tq_tester
{
    class task_thread_manager;

    class task_manager
    {
	public:

	task_manager(libtq::task_queue * const queue);
	~task_manager();

	void add_task(const std::string& label);

	void start_tasks();
	void stop_tasks();

	void print_stats();

	private:

	libtq::task_queue* m_queue;
	std::vector<task_thread_manager*> m_tasks;
    };
}

#endif
