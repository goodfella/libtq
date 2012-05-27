#ifndef TASK_CLEANUP_HPP
#define TASK_CLEANUP_HPP

#include "task_handle.hpp"

namespace libtq
{
    /** Runs the cleanup routine of a task when destroyed
     *
     *  Upon destruction, this class cleans up a task by running the
     *  cleanup routine specified in the constructor.
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
