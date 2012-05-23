#include "bool_flag.hpp"

using namespace tq_tester;

bool_flag::bool_flag():
    m_flag(false)
{
    pthread_mutex_init(&m_lock, NULL);
}

bool_flag::~bool_flag()
{
    pthread_mutex_destroy(&m_lock);
}

void bool_flag::set(const bool val)
{
    pthread_mutex_lock(&m_lock);
    m_flag = val;
    pthread_mutex_unlock(&m_lock);
}

const bool bool_flag::get() const
{
    bool temp;

    pthread_mutex_lock(&m_lock);
    temp = m_flag;
    pthread_mutex_unlock(&m_lock);

    return temp;
}
