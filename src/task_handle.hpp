#ifndef TASK_HANDLE_HPP
#define TASK_HANDLE_HPP

#include <cstring>

namespace libtq
{
    class itask;
    class task;
    class task_allocator;

    /// Smart pointer like object for the task class
    class task_handle
    {
	public:

	task_handle();
	task_handle(task* const taskp, task_allocator* const allocator);
	task_handle(const task_handle& rhs);
	task_handle& operator=(const task_handle& rhs);

	~task_handle();

	task* const operator ->();
	task& operator*();

	/// Determines if a valid task object is referenced
	const bool is_set() const;

	/// Returns true if the underlying task object refers to the given itask
	const bool operator == (itask const * const rhs) const;

	private:

	task* m_task;
	task_allocator* m_allocator;
    };

    inline task* const task_handle::operator ->()
    {
	return m_task;
    }

    inline const bool task_handle::is_set() const
    {
	return m_task != NULL;
    }
}

#endif
