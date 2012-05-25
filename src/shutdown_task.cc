#include "shutdown_task.hpp"

using namespace libtq;

void shutdown_task::run()
{
    throw shutdown_exception();
}
