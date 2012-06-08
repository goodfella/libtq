#include "shutdown_task.hpp"
#include "runner_canceled.hpp"
#include "mutex_lock.hpp"

using namespace libtq;

class signal_finished
{
    public:

    signal_finished(pthread_cond_t* cond, pthread_mutex_t* m, bool* predicate);
    ~signal_finished();

    private:

    // no copying
    signal_finished(const signal_finished& rhs);
    signal_finished& operator=(const signal_finished& rhs);

    pthread_cond_t* m_condition;
    pthread_mutex_t* m_mutex;
    bool* m_predicate;
};

signal_finished::signal_finished(pthread_cond_t* c, pthread_mutex_t* m, bool* p):
    m_condition(c),
    m_mutex(m),
    m_predicate(p)
{}

signal_finished::~signal_finished()
{
    {
	mutex_lock lock(m_mutex);
	*m_predicate = true;
    }

    pthread_cond_signal(m_condition);
}

shutdown_task::shutdown_task():
    m_finished(false)
{
    pthread_cond_init(&m_cond, NULL);
    pthread_mutex_init(&m_lock, NULL);
}

shutdown_task::~shutdown_task()
{
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_cond);
}

void shutdown_task::run()
{
    signal_finished(&m_cond, &m_lock, &m_finished);
    throw runner_canceled();
}

void shutdown_task::wait_for_completion()
{
    mutex_lock lock(&m_lock);
    
    while( m_finished == false )
    {
	pthread_cond_wait(&m_cond, &m_lock);
    }
}
