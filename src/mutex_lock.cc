#include "mutex_lock.hpp"
using namespace libtq;

mutex_lock::mutex_lock(pthread_mutex_t* lock):
    m_lock(lock)
{
    this->lock(lock);
}

mutex_lock::~mutex_lock()
{
    unlock();
}

void mutex_lock::unlock()
{
    if( m_lock != NULL )
    {
	pthread_mutex_unlock(m_lock);
	m_lock = NULL;
    }
}

void mutex_lock::lock(pthread_mutex_t* lock)
{
    if( lock != NULL )
    {
	pthread_mutex_lock(lock);
    }
}
