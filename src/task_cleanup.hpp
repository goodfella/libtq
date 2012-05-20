#ifndef TASK_CLEANUP_HPP
#define TASK_CLEANUP_HPP

#include "task_handle.hpp"

namespace libtq
{
    /** Insures there are no more references to a task's task_handle
     *
     * This class is used when all task_handle references to a task
     * need to be cleaned up.  It signals the waiters that the task is
     * finished, and waits for the waiters to relinquish their
     * task_handles.
     *
     * @note There shall only be one task_cleanup object for a task at
     * any time.  Otherwise each thread with a task_cleanup object for
     * the same task will block indefinitely.
     */
    class task_cleanup
    {
	public:

	task_cleanup(void (task::*signaler)(void));
	task_cleanup(const task_handle& rhs, void (task::*signaler)(void));
	~task_cleanup();

	task_cleanup& operator=(const task_handle& rhs);

	task * const operator->();
	task& operator*();

	private:

	task_cleanup(const task_cleanup& rhs);
	task_cleanup& operator=(const task_cleanup& rhs);

	void (task::*m_signaler)(void);
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
