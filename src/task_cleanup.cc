#include "task_cleanup.hpp"
#include "task.hpp"
using namespace libtq;

task_cleanup::task_cleanup(const task_handle& rhs):
    m_handle(rhs)
{}

task_cleanup::~task_cleanup()
{
    if( m_handle.is_set() )
    {
	m_handle->signal_finished();
    }
}
