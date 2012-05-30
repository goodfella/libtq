#ifndef SHUTDOWN_TASK_HPP
#define SHUTDOWN_TASK_HPP

#include "itask.hpp"

namespace libtq
{
    /// Task class that throws the shutdown exception
    struct shutdown_task : public itask
    {
	void run();
    };
}

#endif
