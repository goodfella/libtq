#ifndef TASK_CLEANUP_HPP
#define TASK_CLEANUP_HPP

#include "task_handle.hpp"

namespace libtq
{
    /// Used to run a task and notify waiters that the task is finished
    class task_cleanup
    {
	public:

	task_cleanup(const task_handle& rhs);

	/// Notifies the waiters that the task is complete
	~task_cleanup();

	task_cleanup& operator=(const task_handle& rhs);

	task * const operator->();
	task& operator*();

	private:

	task_cleanup(const task_cleanup& rhs);
	task_cleanup& operator=(const task_cleanup& rhs);

	task_handle m_handle;
    };

    inline task* const task_cleanup::operator->()
    {
	return &(*m_handle);
    }

    inline task& task_cleanup::operator*()
    {
	return *m_handle;
    }

    inline task_cleanup& task_cleanup::operator=(const task_handle& rhs)
    {
	m_handle = rhs;
	return *this;
    }
}

#endif
