#ifndef MUTEX_LOCK_HPP
#define MUTEX_LOCK_HPP

#include <pthread.h>

namespace libtq
{
    /// Provides RAII locking for a pthread mutex
    class mutex_lock
    {
	public:

	mutex_lock(pthread_mutex_t* lock);
	~mutex_lock();

	private:

	// no copying allowed
	mutex_lock(const mutex_lock& rhs);
	mutex_lock& operator=(const mutex_lock& rhs);

	pthread_mutex_t* m_lock;
    };

    inline mutex_lock::mutex_lock(pthread_mutex_t* lock):
	m_lock(lock)
    {
	pthread_mutex_lock(m_lock);
    }

    inline mutex_lock::~mutex_lock()
    {
	pthread_mutex_unlock(m_lock);
    }
}

#endif
