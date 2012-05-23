#ifndef BOOL_FLAG_HPP
#define BOOL_FLAG_HPP

#include <pthread.h>

namespace tq_tester
{
    class bool_flag
    {
	public:

	bool_flag();
	~bool_flag();

	void set(bool val);
	const bool get() const;

	private:

	bool m_flag;
	mutable pthread_rwlock_t m_lock;
    };
}

#endif
