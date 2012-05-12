#include <stdexcept>
#include <memory>
#include <algorithm>
#include <functional>

#include "task_manager.hpp"
#include "task_thread_manager.hpp"
#include "task_queue.hpp"

using namespace std;
using namespace tq_tester;
using namespace libtq;

task_manager::task_manager(task_queue * const queue):
    m_queue(queue)
{}

task_manager::~task_manager()
{
    while( m_tasks.empty() == false )
    {
	// destroy all the task threads and remove them from the list
	delete(m_tasks.back());

	// pop_back has a no throw guarantee
	m_tasks.pop_back();
    }
}

void task_manager::add_task(const string& label)
{
    auto_ptr<task_thread_manager> taskp(new task_thread_manager(label, m_queue));
    m_tasks.push_back(taskp.get());
    taskp.release();
}

void task_manager::start_tasks()
{
    for_each(m_tasks.begin(), m_tasks.end(),
	     mem_fun(&task_thread_manager::start_threads));
}

void task_manager::stop_tasks()
{
    for_each(m_tasks.begin(), m_tasks.end(),
	     mem_fun(&task_thread_manager::stop_threads));
}

void task_manager::print_stats()
{
	for_each(m_tasks.begin(), m_tasks.end(),
		 mem_fun(&task_thread_manager::print_stats));
}
