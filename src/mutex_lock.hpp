#ifndef MUTEX_LOCK_HPP
#define MUTEX_LOCK_HPP

#include <pthread.h>

namespace libtq
{
    class mutex_lock
    {
	public:

	mutex_lock(pthread_mutex_t* lock);
	~mutex_lock();

	void unlock();
	void lock(pthread_mutex_t* lock);

	private:

	pthread_mutex_t* m_lock;
    };
}

#endif
