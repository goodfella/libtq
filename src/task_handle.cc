#include "task_handle.hpp"
#include "itask.hpp"
#include "task_allocator.hpp"

using namespace libtq;

task_handle::task_handle():
    m_task(NULL),
    m_allocator(NULL)
{}

task_handle::task_handle(task * const taskp, task_allocator* allocator):
    m_task(taskp),
    m_allocator(allocator)
{
    m_task->get_ref();
}

task_handle::task_handle(const task_handle& rhs):
    m_task(rhs.m_task),
    m_allocator(rhs.m_allocator)
{
    if( m_task != NULL )
    {
	m_task->get_ref();
    }
}

task_handle& task_handle::operator=(const task_handle& rhs)
{
    if( this == &rhs )
    {
	// self assignment just return this
	return *this;
    }

    if( m_task != NULL && m_task->put_ref() == 0 )
    {
	m_allocator->free(m_task);
	m_task = NULL;
    }

    m_task = rhs.m_task;
    m_task->get_ref();
    m_allocator = rhs.m_allocator;

    return *this;
}

task_handle::~task_handle()
{
    if( m_task != NULL && m_task->put_ref() == 0 )
    {
	m_allocator->free(m_task);
    }
}

const bool task_handle::operator == (itask const * const rhs) const
{
    return *m_task == rhs;
}

task& task_handle::operator*()
{
    return *m_task;
}
