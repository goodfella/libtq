#include <stdexcept>
#include <memory>
#include <algorithm>
#include <functional>

#include "task_manager.hpp"
#include "task_thread_manager.hpp"
#include "task_queue.hpp"

using namespace tq_tester;
using namespace libtq;

task_manager::task_manager(task_queue * const queue):
    m_queue(queue)
{}

void task_manager::add_task(const std::string& label)
{
    m_tasks.push_back(std::make_unique<task_thread_manager>(label, m_queue));
}

void task_manager::start_tasks()
{
    std::for_each(m_tasks.begin(), m_tasks.end(),
		  std::mem_fn(&task_thread_manager::start_threads));
}

void task_manager::stop_tasks()
{
    std::for_each(m_tasks.begin(), m_tasks.end(),
		  std::mem_fn(&task_thread_manager::stop_threads));
}

void task_manager::print_stats()
{
    std::for_each(m_tasks.begin(), m_tasks.end(),
		  std::mem_fn(&task_thread_manager::print_stats));
}
