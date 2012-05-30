#include "shutdown_task.hpp"
#include "runner_canceled.hpp"

using namespace libtq;

void shutdown_task::run()
{
    throw runner_canceled();
}
