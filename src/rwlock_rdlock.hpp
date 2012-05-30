#ifndef RWLOCK_RDLOCK_HPP
#define RWLOCK_RDLOCK_HPP

namespace libtq
{
    class rwlock_rdlock
    {
	public:

	rwlock_rdlock(pthread_rwlock_t* lock);
	~rwlock_rdlock();
	
	private:

	pthread_rwlock_t* m_lock;
    };

    inline rwlock_rdlock::rwlock_rdlock(pthread_rwlock_t* lock):
	m_lock(lock)
    {
	pthread_rwlock_rdlock(lock);
    }

    inline rwlock_rdlock::~rwlock_rdlock()
    {
	pthread_rwlock_unlock(m_lock);
    }
}

#endif
