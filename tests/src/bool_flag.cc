#include "bool_flag.hpp"

using namespace tq_tester;

bool_flag::bool_flag()
{
    m_flag.store(false);
}

void bool_flag::set(const bool val)
{
    m_flag.store(val);
}

const bool bool_flag::get() const
{
    return m_flag.load();
}
