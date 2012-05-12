#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include <functional>
#include <stdexcept>

#include "task_queue.hpp"
#include "task_thread_manager.hpp"
#include "queue_thread_manager.hpp"

using namespace std;
using namespace libtq;
using namespace tq_tester;

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
