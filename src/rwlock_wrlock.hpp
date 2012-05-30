#ifndef RWLOCK_WRLOCK_HPP
#define RWLOCK_WRLOCK_HPP

namespace libtq
{
    class rwlock_wrlock
    {
	public:

	rwlock_wrlock(pthread_rwlock_t* lock);
	~rwlock_wrlock();
	
	private:

	pthread_rwlock_t* m_lock;
    };

    inline rwlock_wrlock::rwlock_wrlock(pthread_rwlock_t* lock):
	m_lock(lock)
    {
	pthread_rwlock_wrlock(lock);
    }

    inline rwlock_wrlock::~rwlock_wrlock()
    {
	pthread_rwlock_unlock(m_lock);
    }
}

#endif
