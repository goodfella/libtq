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
}

#endif
