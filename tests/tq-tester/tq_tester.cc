#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>
#include <stdexcept>

#include "task_queue.hpp"
#include "queue_thread_manager.hpp"
#include "task_manager.hpp"

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

    try
    {
	task_manager tasks(&queue);
	queue_thread_manager queue_thread(&queue);

	for( int i = 1; i <= num_task_threads; ++i)
	{
	    stringstream ss;
	    ss << "task " << i;
	    tasks.add_task(ss.str());
	}

	tasks.start_tasks();
	queue_thread.start_threads();
	
	cout << "press enter to stop test: ";
	char input;
	cin.get(input);
	cout << "received input stopping threads\n";

	queue_thread.stop_threads();

	// stops and joins on all the task threads
	tasks.stop_tasks();
	tasks.print_stats();

	cout << "exiting program\n";
    }
    catch(std::exception& ex)
    {
	cerr << ex.what() << endl;
	return 1;
    }

    return 0;
}
