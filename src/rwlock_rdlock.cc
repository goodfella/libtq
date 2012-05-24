#include <stdexcept>
#include "rwlock_rdlock.hpp"

using namespace libtq;

rwlock_rdlock::rwlock_rdlock(pthread_rwlock_t* lock):
    m_lock(lock)
{
    if( lock == NULL )
    {
	throw std::logic_error("rwlock_rdlock constructor lock parameter is NULL");
    }

    pthread_rwlock_rdlock(lock);
}

rwlock_rdlock::~rwlock_rdlock()
{
    pthread_rwlock_unlock(m_lock);
}
