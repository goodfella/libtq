#ifndef RUNNER_CANCELED_HPP
#define RUNNER_CANCELED_HPP

#include <stdexcept>

namespace libtq
{
    struct runner_canceled: public std::exception
    {
	const char* what() const throw();
    };
}

#endif
