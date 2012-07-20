#ifndef RUNNER_CANCELED_HPP
#define RUNNER_CANCELED_HPP

#include <stdexcept>

namespace libtq
{
    /// Exception thrown when a task runner thread needs to exit
    struct runner_canceled: public std::exception
    {
	const char* what() const throw();
    };
}

#endif
