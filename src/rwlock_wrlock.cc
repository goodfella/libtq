#include <stdexcept>
#include "rwlock_wrlock.hpp"

using namespace libtq;

rwlock_wrlock::rwlock_wrlock(pthread_rwlock_t* lock):
    m_lock(lock)
{
    if( lock == NULL )
    {
	throw std::logic_error("rwlock_wrlock constructor lock parameter is NULL");
    }

    pthread_rwlock_wrlock(lock);
}

rwlock_wrlock::~rwlock_wrlock()
{
    pthread_rwlock_unlock(m_lock);
}
