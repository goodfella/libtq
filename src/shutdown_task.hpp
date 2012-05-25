#ifndef SHUTDOWN_TASK_HPP
#define SHUTDOWN_TASK_HPP

#include "itask.hpp"

namespace libtq
{
    /// Exception that's thrown to shutdown a task runner thread
    struct shutdown_exception {};

    /// Task class that throws the shutdown exception
    struct shutdown_task : public itask
    {
	void run();
    };
}

#endif
