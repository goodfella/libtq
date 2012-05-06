#include <pthread.h>
#include <signal.h>
#include <iostream>

#include "task_queue.hpp"

using namespace std;
using namespace libtq;

task_queue queue;

void* stop_handler(void*)
{
    sigset_t sigmask;

    sigfillset(&sigmask);

    // block all signals
    if( pthread_sigmask(SIG_BLOCK, &sigmask, NULL) )
    {
	cerr << "error blocking signals in stop_handle\n";
	pthread_exit(NULL);
    }

    cout << "press enter to stop test: ";
    char input;
    cin.get(input);
    cout << "received input exiting stop_handler\n";
    
    pthread_exit(NULL);
}

void queue_handler_cancel(void* p)
{
    cout << "queue_handler canceled\n";
}

bool exit_queue_handler;
pthread_mutex_t exit_queue_handler_lock;

void* queue_handler(void* q)
{
    task_queue* queue = static_cast<task_queue*>(q);

    pthread_mutex_lock(&exit_queue_handler_lock);

    while ( exit_queue_handler == false )
    {
	if( queue->start_queue() )
	{
	    cerr << "error starting task queue\n";
	    break;
	}

	pthread_mutex_unlock(&exit_queue_handler_lock);
	pthread_yield();

	if( queue->stop_queue() )
	{
	    cerr << "error stopping queue\n";
	    pthread_exit(NULL);
	}

	pthread_mutex_lock(&exit_queue_handler_lock);
    }

    pthread_mutex_unlock(&exit_queue_handler_lock);
    pthread_exit(NULL);
}

int main()
{
    int rc = 0;
    pthread_t stop_thread;
    pthread_t queue_thread;

    pthread_mutex_init(&exit_queue_handler_lock, NULL);
    queue.start_queue();

    if( pthread_create(&stop_thread, NULL, stop_handler, NULL) )
    {
	cerr << "error starting stop_handler\n";
	return 1;
    }

    if( pthread_create(&queue_thread, NULL, queue_handler, &queue) )
    {
    	cerr << "error creating queue thread\n";
    	pthread_cancel(stop_thread);
    	pthread_join(stop_thread, NULL);
    	return 1;
    }

    // wait for the stop thread to finish
    pthread_join(stop_thread, NULL);

    // shutdown the queue thread
    pthread_mutex_lock(&exit_queue_handler_lock);
    exit_queue_handler = true;
    pthread_mutex_unlock(&exit_queue_handler_lock);

    cout << "joining the queue thread\n";
    pthread_join(queue_thread, NULL);

    queue.stop_queue();

    cout << "exiting program\n";   

    return rc;
}
