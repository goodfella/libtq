#include "shutdown_task.hpp"
#include "runner_canceled.hpp"

using namespace libtq;

void shutdown_task::wait_task_run()
{
    throw runner_canceled();
}
