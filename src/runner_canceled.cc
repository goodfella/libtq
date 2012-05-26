#include "runner_canceled.hpp"

using namespace libtq;

const char* runner_canceled::what() const throw()
{
    return "task runner canceled";
}
