#ifndef SHUTDOWN_TASK_HPP
#define SHUTDOWN_TASK_HPP

#include <pthread.h>
#include "itask.hpp"

namespace libtq
{
    /// Task class that throws the shutdown exception
    class shutdown_task : public itask
    {
	public:

	shutdown_task();
	~shutdown_task();

	void run();
	void wait_for_completion();

	private:

	// no copying allowed
	shutdown_task(const shutdown_task& rhs);
	shutdown_task& operator=(const shutdown_task& rhs);

	bool m_finished;
	pthread_mutex_t m_lock;
	pthread_cond_t m_cond;
    };
}

#endif
