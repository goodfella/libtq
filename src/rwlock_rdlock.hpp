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
}

#endif
