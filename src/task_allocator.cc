#include "task_allocator.hpp"
#include "mutex_lock.hpp"

using namespace libtq;

task_allocator::task_allocator():
    m_head(NULL),
    m_chunks(NULL)
{
    pthread_mutex_init(&m_lock, NULL);
}

task_allocator::~task_allocator()
{
    {
	mutex_lock lock(&m_lock);

	chunk* next = m_chunks;
	
	while( next != NULL )
	{
	    chunk* p = next;
	    next = next->next;
	    delete p;
	}
    }

    pthread_mutex_destroy(&m_lock);
}

task* const task_allocator::alloc(itask* const taskp)
{
    mutex_lock lock(&m_lock);

    if( m_head == NULL )
    {
	grow();
    }

    task* new_task = m_head;

    // new_task->itaskp returns the next task*
    m_head = new_task->itaskp(taskp);

    return new_task;
}

void task_allocator::free(task* const taskp)
{
    mutex_lock lock(&m_lock);

    taskp->reset(m_head);
    m_head = taskp;
}

void task_allocator::grow()
{
    chunk* new_chunk = new chunk();
    
    new_chunk->next = m_chunks;
    m_chunks = new_chunk;
    m_head = new_chunk->tasks;
}

task_allocator::chunk::chunk()
{
    for(int i = 0; i < chunk::size - 1; ++i)
    {
	tasks[i].reset(&tasks[i + 1]);
    }

    tasks[chunk::size - 1].reset(NULL);
}
