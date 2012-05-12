#include <pthread.h>
#include <signal.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <string>
#include <algorithm>
#include <functional>

#include "task_queue.hpp"
#include "itask.hpp"

using namespace std;
using namespace libtq;

class task : public itask
{
    public:

    task();
    ~task();

    void run();
    const unsigned long runcount() const;
    const unsigned long cancelcount() const;
    void inc_waitcount();
    void inc_cancelcount();

    private:

    void inc_runcount();
    void inc_counter(unsigned long& counter);
    const unsigned long counter(const unsigned long& counter) const;

    mutable pthread_mutex_t m_lock;
    unsigned long m_runcount;
    unsigned long m_waitcount;
    unsigned long m_cancelcount;
};

task::task():
    m_runcount(0),
    m_waitcount(0),
    m_cancelcount(0)
{
    pthread_mutex_init(&m_lock, NULL);
}

task::~task()
{
    pthread_mutex_destroy(&m_lock);
}

void task::inc_counter(unsigned long& counter)
{
    pthread_mutex_lock(&m_lock);
    ++counter;
    pthread_mutex_unlock(&m_lock);
}

void task::inc_runcount()
{
    inc_counter(m_runcount);
}

const unsigned long task::counter(const unsigned long& counter) const
{
    unsigned long temp;
    pthread_mutex_lock(&m_lock);
    temp = counter;
    pthread_mutex_unlock(&m_lock);
    return temp;
}

const unsigned long task::runcount() const
{
    return counter(m_runcount);
}

const unsigned long task::cancelcount() const
{
    return counter(m_cancelcount);
}

void task::inc_waitcount()
{
    inc_counter(m_waitcount);
}

void task::inc_cancelcount()
{
    inc_counter(m_cancelcount);
}

void task::run()
{
    inc_runcount();
}

class bool_flag
{
    public:

    bool_flag();
    ~bool_flag();

    void set(bool val);
    const bool get() const;

    private:

    bool m_flag;
    mutable pthread_mutex_t m_lock;
};

bool_flag::bool_flag():
    m_flag(false)
{
    pthread_mutex_init(&m_lock, NULL);
}

bool_flag::~bool_flag()
{
    pthread_mutex_destroy(&m_lock);
}

void bool_flag::set(const bool val)
{
    pthread_mutex_lock(&m_lock);
    m_flag = val;
    pthread_mutex_unlock(&m_lock);
}

const bool bool_flag::get() const
{
    bool temp;

    pthread_mutex_lock(&m_lock);
    temp = m_flag;
    pthread_mutex_unlock(&m_lock);

    return temp;
}

struct task_thread_data
{
    task_thread_data(task* t, task_queue* const q, bool_flag* f);

    task* taskp;
    task_queue* queue;
    bool_flag* stop_thread;
};

task_thread_data::task_thread_data(task* t, task_queue* const q, bool_flag* f):
    taskp(t),
    queue(q),
    stop_thread(f)
{}

class task_thread_manager
{
    public:

    task_thread_manager(const string& label, task_queue * const queue);
    ~task_thread_manager();

    void start_threads();
    void stop_threads();
    void print_stats() const;

    private:

    task m_task;
    bool_flag m_stop_threads;
    string m_label;
    task_thread_data m_desc;

    bool m_sch_started;
    bool m_sch_wait_started;
    bool m_cancel_started;
    bool m_wait_started;

    pthread_t m_task_sch;
    pthread_t m_task_sch_wait;
    pthread_t m_task_cancel;
    pthread_t m_task_wait;

    static void* task_sch_handler(void* task);
    static void* task_sch_wait_handler(void* task);
    static void* task_cancel_handler(void* task);
    static void* task_wait_handler(void* task);
};

task_thread_manager::task_thread_manager(const string& label, task_queue * const queue):
    m_label(label),
    m_desc(&m_task, queue, &m_stop_threads),
    m_sch_started(false),
    m_sch_wait_started(false),
    m_cancel_started(false),
    m_wait_started(false)
{}

void task_thread_manager::start_threads()
{
    if( pthread_create(&m_task_sch, NULL, task_thread_manager::task_sch_handler, &m_desc) != 0 )
    {
	throw std::runtime_error("error creating task scheduler thread");
    }

    m_sch_started = true;

    if( pthread_create(&m_task_sch_wait, NULL, task_thread_manager::task_sch_wait_handler, &m_desc) != 0 )
    {
	throw std::runtime_error("error creating task schedule with a wait thread");
    }

    m_sch_wait_started = true;

    if( pthread_create(&m_task_cancel, NULL, task_thread_manager::task_cancel_handler, &m_desc) != 0 )
    {
	throw std::runtime_error("error creating task canceler thread");
    }

    m_cancel_started = true;

    if( pthread_create(&m_task_wait, NULL, task_thread_manager::task_wait_handler, &m_desc) != 0 )
    {
	throw std::runtime_error("error creating task wait thread");
    }

    m_wait_started = true;
}

void task_thread_manager::stop_threads()
{
    if( m_sch_started == false &&
	m_sch_wait_started == false &&
	m_wait_started == false &&
	m_cancel_started == false )
    {
	return;
    }

    // make sure the queue is started so the threads can clean up
    m_desc.queue->start_queue();

    // stop all the threads
    m_stop_threads.set(true);

    if( m_sch_started == true )
    {
	pthread_join(m_task_sch, NULL);
	m_sch_started = false;
    }

    if( m_sch_wait_started == true )
    {
	pthread_join(m_task_sch_wait, NULL);
	m_sch_wait_started = false;
    }

    if( m_wait_started == true )
    {
	pthread_join(m_task_wait, NULL);
	m_wait_started = false;
    }

    if( m_cancel_started == true )
    {
	pthread_join(m_task_cancel, NULL);
	m_cancel_started = false;
    }
}

task_thread_manager::~task_thread_manager()
{
    stop_threads();
}

void task_thread_manager::print_stats() const
{
    cout << m_label << " run count = " << m_task.runcount() << endl
	 << m_label << " cancel count = " << m_task.cancelcount() << endl << endl;
}

void* task_thread_manager::task_sch_handler(void* t)
{
    task_thread_data* desc = static_cast<task_thread_data*>(t);

    while( desc->stop_thread->get() == false )
    {
	desc->queue->queue_task(desc->taskp);
	pthread_yield();
    };

    pthread_exit(NULL);
}

void* task_thread_manager::task_sch_wait_handler(void* t)
{
    task_thread_data* desc = static_cast<task_thread_data*>(t);

    while( desc->stop_thread->get() == false )
    {
	desc->queue->queue_task(desc->taskp);
	desc->queue->wait_for_task(desc->taskp);
	pthread_yield();
    };

    pthread_exit(NULL);
}

void* task_thread_manager::task_cancel_handler(void* t)
{
    task_thread_data* desc = static_cast<task_thread_data*>(t);

    while( desc->stop_thread->get() == false )
    {
	if( desc->queue->cancel_task(desc->taskp) == true )
	{
	    desc->taskp->inc_cancelcount();
	}

	pthread_yield();
    }

    pthread_exit(NULL);
}

void* task_thread_manager::task_wait_handler(void* t)
{
    task_thread_data* desc = static_cast<task_thread_data*>(t);

    while( desc->stop_thread->get() == false )
    {
	desc->queue->wait_for_task(desc->taskp);
	pthread_yield();
    };

    pthread_exit(NULL);
}

class queue_thread_manager
{
    public:

    queue_thread_manager(task_queue* const queue);
    ~queue_thread_manager();

    void start_thread();
    void stop_thread();

    private:

    task_queue* m_queue;
    bool m_thread_started;

    pthread_t m_thread;

    static bool_flag m_stop_thread;
    static void* queue_thread(void*);
};

queue_thread_manager::queue_thread_manager(task_queue* const queue):
    m_queue(queue),
    m_thread_started(false)
{}

queue_thread_manager::~queue_thread_manager()
{
    stop_thread();
}

void queue_thread_manager::start_thread()
{
    if( pthread_create(&m_thread, NULL, &queue_thread_manager::queue_thread, m_queue) != 0 )
    {
	throw std::runtime_error("error running queue thread");
    }

    m_thread_started = true;
}

void queue_thread_manager::stop_thread()
{
    if( m_thread_started == false )
    {
	return;
    }

    m_stop_thread.set(true);

    if( m_thread_started == true )
    {
	pthread_join(m_thread, NULL);
	m_thread_started = false;
    }
}

bool_flag queue_thread_manager::m_stop_thread;

void* queue_thread_manager::queue_thread(void* q)
{
    task_queue * queue = static_cast<task_queue*>(q);

    while ( m_stop_thread.get() == false )
    {
	if( queue->start_queue() )
	{
	    cerr << "error starting task queue\n";
	    break;
	}

	pthread_yield();

	queue->stop_queue();
    }

    pthread_exit(NULL);
}

int main(int argc, char** argv)
{
    task_queue queue;

    if( argc < 2 )
    {
	cerr << "Usage: tq-tester <num-task-threads>\n";
	return 1;
    }

    int num_task_threads = atoi(argv[1]);

    if( num_task_threads == 0 )
    {
	cerr << "Invalid number of task threads: " << num_task_threads << endl;
	return 1;
    }

    vector<task_thread_manager*> task_threads;

    try
    {
	queue_thread_manager queue_thread(&queue);

	for( int i = 1; i <= num_task_threads; ++i)
	{
	    stringstream ss;
	    ss << "task " << i;
	    task_threads.push_back(new task_thread_manager(ss.str(), &queue));
	}

	// start each thread
	for_each(task_threads.begin(), task_threads.end(),
		 mem_fun(&task_thread_manager::start_threads));

	queue_thread.start_thread();
	
	cout << "press enter to stop test: ";
	char input;
	cin.get(input);
	cout << "received input stopping threads\n";

	queue_thread.stop_thread();

	// stops and joins on all the task threads
	for_each(task_threads.begin(), task_threads.end(),
		 mem_fun(&task_thread_manager::stop_threads));

	// prints the thread stats
	for_each(task_threads.begin(), task_threads.end(),
		 mem_fun(&task_thread_manager::print_stats));

	cout << "exiting program\n";
    }
    catch(std::runtime_error& ex)
    {
	while( task_threads.empty() == false )
	{
	    // destroy all the task threads and remove them from teh list
	    delete(task_threads.back());
	    task_threads.pop_back();
	}

	cerr << ex.what() << endl;
	return 1;
    }

    return 0;
}
