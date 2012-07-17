#ifndef TASK_THREAD_MANAGER_HPP
#define TASK_THREAD_MANAGER_HPP

#include <string>

#include "bool_flag.hpp"
#include "test_task.hpp"
#include "thread.hpp"

namespace libtq
{
    class task_queue;
}

namespace tq_tester
{
    struct task_thread_data
    {
	task_thread_data(test_task* t, libtq::task_queue* const q, bool_flag* f);

	test_task* taskp;
	libtq::task_queue* queue;
	bool_flag* stop_thread;
    };

    class task_thread_manager
    {
	public:

	task_thread_manager(const std::string& label, libtq::task_queue * const queue);
	~task_thread_manager();

	void start_threads();
	void stop_threads();
	void print_stats() const;

	private:

	test_task m_task;
	bool_flag m_stop_threads;
	std::string m_label;
	task_thread_data m_desc;

	libtq::thread m_sch_thread;
	libtq::thread m_scheduler_thread;
	libtq::thread m_wait_thread;

	static void* task_sch_handler(void* task);
	static void* task_scheduler(void* data);
	static void* wait_handler(void* data);
    };
}

#endif
