#include "test_task.hpp"

using namespace tq_tester;
using namespace libtq;

test_task::test_task():
    m_runcount(0),
    m_cancelcount(0),
    m_waitcount(0)
{
    pthread_mutex_init(&m_lock, NULL);
}

test_task::~test_task()
{
    pthread_mutex_destroy(&m_lock);
}

void test_task::inc_counter(unsigned long& counter)
{
    pthread_mutex_lock(&m_lock);
    ++counter;
    pthread_mutex_unlock(&m_lock);
}

const unsigned long test_task::counter(const unsigned long& counter) const
{
    unsigned long temp;
    pthread_mutex_lock(&m_lock);
    temp = counter;
    pthread_mutex_unlock(&m_lock);
    return temp;
}

const unsigned long test_task::runcount() const
{
    return counter(m_runcount);
}

const unsigned long test_task::cancelcount() const
{
    return counter(m_cancelcount);
}

const unsigned long test_task::waitcount() const
{
    return counter(m_waitcount);
}

void test_task::run()
{
    wait_task::signaler cleanup(this);
    inc_counter(m_runcount);
}

void test_task::canceled()
{
    wait_task::signaler cleanup(this);
    inc_counter(m_cancelcount);
}

void test_task::wait()
{
    inc_counter(m_waitcount);
    wait_task::wait();
}
