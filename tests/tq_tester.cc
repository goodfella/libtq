#include <pthread.h>
#include <signal.h>
#include <iostream>

#include "task_queue.hpp"

using namespace std;
using namespace libtq;

task_queue queue;

// predicate the threads use to determine if they need to be stopped
bool stop_threads;

// lock for the stop_threads predicate
pthread_mutex_t stop_threads_lock;

// waits for input to stop the tester
pthread_t stop_thread;

// starts and stops the queue
pthread_t queue_thread;

// sends signals to all the threads
pthread_t signal_thread;

void* stop_handler(void*)
{
    cout << "press enter to stop test: ";
    char input;
    cin.get(input);
    cout << "received input exiting stop_handler\n";
    
    pthread_exit(NULL);
}

void* queue_handler(void* q)
{
    task_queue* queue = static_cast<task_queue*>(q);

    pthread_mutex_lock(&stop_threads_lock);

    while ( stop_threads == false )
    {
	if( queue->start_queue() )
	{
	    cerr << "error starting task queue\n";
	    break;
	}

	pthread_mutex_unlock(&stop_threads_lock);
	pthread_yield();

	if( queue->stop_queue() )
	{
	    cerr << "error stopping queue\n";
	    pthread_exit(NULL);
	}

	pthread_mutex_lock(&stop_threads_lock);
    }

    pthread_mutex_unlock(&stop_threads_lock);
    pthread_exit(NULL);
}

int main()
{
    int rc = 0;

    pthread_mutex_init(&stop_threads_lock, NULL);
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
    pthread_mutex_lock(&stop_threads_lock);
    stop_threads = true;
    pthread_mutex_unlock(&stop_threads_lock);

    cout << "joining the queue thread\n";
    pthread_join(queue_thread, NULL);

    queue.stop_queue();

    cout << "exiting program\n";   

    return rc;
}
