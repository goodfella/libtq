#ifndef TASK_ALLOCATOR_HPP
#define TASK_ALLOCATOR_HPP

#include <pthread.h>
#include "task.hpp"

namespace libtq
{
    class itask;

    /// Allocator for the task class
    class task_allocator
    {
	public:

	task_allocator();
	~task_allocator();

	/// Returns a task pointer and sets its itask pointer
	task* const alloc(itask * const itaskp);

	/// Returns a task pointer to the allocator and resets it
	void free(task * const taskp);

	private:

	void grow();

	struct chunk
	{
	    chunk();

	    enum { size = 16 };
	    task tasks[size];
	    chunk* next;
	};

	task* m_head;
	chunk* m_chunks;
	pthread_mutex_t m_lock;
    };
}

#endif
