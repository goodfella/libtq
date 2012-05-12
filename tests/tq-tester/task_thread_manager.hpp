#ifndef TASK_THREAD_MANAGER_HPP
#define TASK_THREAD_MANAGER_HPP

#include <string>

#include "bool_flag.hpp"
#include "task.hpp"

namespace libtq
{
    class task_queue;
}

namespace tq_tester
{
    struct task_thread_data
    {
	task_thread_data(task* t, libtq::task_queue* const q, bool_flag* f);

	task* taskp;
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

	task m_task;
	bool_flag m_stop_threads;
	std::string m_label;
	task_thread_data m_desc;

	bool m_sch_started;
	bool m_sch_wait_started;
	bool m_cancel_started;
	bool m_wait_started;

	pthread_t m_task_sch;
	pthread_t m_task_sch_wait;
	pthread_t m_task_cancel;
	pthread_t m_task_wait;

	static void* task_sch_handler(void* task);
	static void* task_sch_wait_handler(void* task);
	static void* task_cancel_handler(void* task);
	static void* task_wait_handler(void* task);
    };
}

#endif