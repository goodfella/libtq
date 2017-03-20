#include <mutex>
#include "test_task.hpp"

using namespace tq_tester;
using namespace libtq;

test_task::test_task():
    m_runcount(0),
    m_waitcount(0)
{}

void test_task::inc_counter(unsigned long& counter)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    ++counter;
}

const unsigned long test_task::counter(const unsigned long& counter) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return counter;
}

const unsigned long test_task::runcount() const
{
    return counter(m_runcount);
}

const unsigned long test_task::waitcount() const
{
    return counter(m_waitcount);
}

void test_task::wait_task_run()
{
    inc_counter(m_runcount);
}

void test_task::wait_task_wait()
{
    inc_counter(m_waitcount);
}
