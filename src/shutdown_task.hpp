#ifndef SHUTDOWN_TASK_HPP
#define SHUTDOWN_TASK_HPP

#include "wait_task.hpp"

namespace libtq
{
    /// Task class that throws the shutdown exception
    class shutdown_task : public wait_task
    {
	public:

	void wait_task_run() override;
    };
}

#endif
