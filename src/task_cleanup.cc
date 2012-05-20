#include "task_cleanup.hpp"
#include "task.hpp"
using namespace libtq;

task_cleanup::task_cleanup(void (task::*signaler)(void)):
    m_signaler(signaler)
 {}

task_cleanup::task_cleanup(const task_handle& rhs, void (task::*signaler)(void)):
    m_signaler(signaler),
    m_handle(rhs)
{}

task_cleanup::~task_cleanup()
{
    if( m_handle.is_set() )
    {
	((*m_handle).*m_signaler)();
	m_handle->wait_for_waiters();
    }
}
