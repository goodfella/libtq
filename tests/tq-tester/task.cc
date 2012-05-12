#include "task.hpp"

using namespace tq_tester;

task::task():
    m_runcount(0),
    m_waitcount(0),
    m_cancelcount(0)
{
    pthread_mutex_init(&m_lock, NULL);
}

task::~task()
{
    pthread_mutex_destroy(&m_lock);
}

void task::inc_counter(unsigned long& counter)
{
    pthread_mutex_lock(&m_lock);
    ++counter;
    pthread_mutex_unlock(&m_lock);
}

void task::inc_runcount()
{
    inc_counter(m_runcount);
}

const unsigned long task::counter(const unsigned long& counter) const
{
    unsigned long temp;
    pthread_mutex_lock(&m_lock);
    temp = counter;
    pthread_mutex_unlock(&m_lock);
    return temp;
}

const unsigned long task::runcount() const
{
    return counter(m_runcount);
}

const unsigned long task::cancelcount() const
{
    return counter(m_cancelcount);
}

void task::inc_waitcount()
{
    inc_counter(m_waitcount);
}

void task::inc_cancelcount()
{
    inc_counter(m_cancelcount);
}

void task::run()
{
    inc_runcount();
}
