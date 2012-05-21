#ifndef THREAD_HPP
#define THREAD_HPP

#include <pthread.h>
#include <string>

namespace libtq
{
    class thread
    {
	public:

	thread();

	void start(void* (*thread_func)(void*), void* arg,
		   const std::string& name);

	void join(void** retval);
	void join();

	private:

	bool m_started;
	pthread_t m_thread;
    };
}

#endif
