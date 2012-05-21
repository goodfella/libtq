#include <stdexcept>

#include "thread.hpp"

using namespace libtq;

thread::thread():
    m_started(false)
{}

void thread::start(void* (*thread_func)(void*),
			   void* arg,
			   const std::string& name)
{
    if( m_started == true )
    {
	return;
    }

    if( pthread_create(&m_thread, NULL, thread_func, arg) != 0 )
    {
	throw std::runtime_error("error running: " + name);
    }

    m_started = true;
}

void thread::join(void** retval)
{
    if( m_started == true )
    {
	pthread_join(m_thread, retval);
	m_started = false;
    }
}

void thread::join()
{
    join(NULL);
}
