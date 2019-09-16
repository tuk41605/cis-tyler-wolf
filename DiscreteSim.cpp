#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include "Event.h"

int gTime = 0; // keeps track of the simulation time
int gpId = 0;  // used to generate unique process ids
int gProcessNum = 1; // used to track total process numbers

void handle_process_arrival(Event, std::vector<Event>*, std::vector<Process>*, bool*);
void handle_process_cpu_arrival(Event, std::vector<Event>*, std::vector<Process>*, bool*);
void handle_process_exit_cpu(Event, std::vector<Event>*, std::vector<Process>*, std::vector<Process>*, std::vector<Process>*, bool*, bool*, bool*);
void handle_process_exit_system(Event, std::vector<Event>*, std::vector<Process>*);
void handle_process_arrive_disk1(Event, std::vector<Event>*, std::vector<Process>*, bool*);
void handle_process_arrive_disk2(Event, std::vector<Event>*, std::vector<Process>*, bool*);
void handle_process_finish_disk1(Event, std::vector<Event>*, std::vector<Process>*, std::vector<Process>*, bool*, bool*); 
void handle_process_finish_disk2(Event, std::vector<Event>*, std::vector<Process>*, std::vector<Process>*, bool*, bool*); 
void handle_simulation_finish(Event, std::vector<Event>*); 
void rearrange_priority_queue(std::vector<Event>*, Event);
int get_disk_time();
int get_random_interval();
int get_cpu_interval();
int get_queue_time(std::vector<Event>*, int);
bool check_close();
bool get_exit_probability();
void print_priority_queue(std::vector<Event>*);
void swap(Event*, Event*);

int main()
{
	int seed;
	std::ifstream config;
	std::string line;

	config.open("CONFIG.txt");

	// lines 31-43 fetch the seed from the config file 
	while (std::getline(config, line))
	{
		std::istringstream iss(line);
		std::string header;
		int fileSeed;
		iss >> header >> fileSeed;

		if (header == "SEED")
		{
			seed = fileSeed;
		}

	} // end seed fetch

	config.close();

	srand(seed);

	bool isRunning = true;
	bool cpuInUse = false;
	bool disk1InUse = false;
	bool disk2InUse = false;

	int interval = get_random_interval();

	std::vector<Process> cpuQueue;
	std::vector<Process> disk1Queue;
	std::vector<Process> disk2Queue;
	std::vector<Event> priorityQueue;

	// creates initial process
	Process proc = Process();
	proc.setProcessID(1);
	proc.setProcessState(Process::IN_QUEUE);
	Event ev = Event(::gTime, Event::PROCESS_ARRIVAL, proc);
	priorityQueue.insert(priorityQueue.begin(), ev);

	print_priority_queue(&priorityQueue);

	while (isRunning == true)
	{
		while (priorityQueue.size() > 0)
		{
			// the printing statements were used as testing
			std::cout << "TIME IS: " << ::gTime << std::endl;
			std::cout << "THERE ARE " << ::gProcessNum << " PROCESSES IN THE SIMULATION\n";
			std::cout << "THERE ARE " << cpuQueue.size() << " PROCESSESS IN THE CPU QUEUE\n";
			std::cout << "THERE ARE " << disk1Queue.size() << " PROCESSES IN THE DISK 1 QUEUE\n";
			std::cout << "THERE ARE " << disk2Queue.size() << " PROCESSES IN THE DISK 2 QUEUE\n";
			std::cout << "CONTINUE? " << std::endl;
			// these statements were used to keep track of the simulation as it progressed
			char p;
			std::cin >> p;
			bool closed = check_close();
			if (closed == true)
			{
				Event handledEvent = Event(::gTime, Event::SIMULATION_FINISH, priorityQueue.back().getProcess());
				rearrange_priority_queue(&priorityQueue, handledEvent);
			}
			else
			{
				Event handledEvent = priorityQueue.back();

				switch (handledEvent.getEventType())
				{
				case Event::PROCESS_ARRIVAL:
					handle_process_arrival(handledEvent, &priorityQueue, &cpuQueue, &cpuInUse);
					break;
				case Event::PROCESS_ARRIVE_CPU:
					handle_process_cpu_arrival(handledEvent, &priorityQueue, &cpuQueue, &cpuInUse);
					break;
				case Event::PROCESS_FINISH_CPU:
					handle_process_exit_cpu(handledEvent, &priorityQueue, &cpuQueue, &disk1Queue, &disk2Queue, &cpuInUse, &disk1InUse, &disk2InUse);
					break;
				case Event::PROCESS_EXIT_SYSTEM:
					handle_process_exit_system(handledEvent, &priorityQueue, &cpuQueue);
					break;
				case Event::PROCESS_ARRIVE_DISK1:
					handle_process_arrive_disk1(handledEvent, &priorityQueue, &disk1Queue, &disk1InUse);
					break;
				case Event::PROCESS_ARRIVE_DISK2:
					handle_process_arrive_disk2(handledEvent, &priorityQueue, &disk2Queue, &disk2InUse);
					break;
				case Event::PROCESS_FINISH_DISK1:
					handle_process_finish_disk1(handledEvent, &priorityQueue, &disk1Queue, &cpuQueue, &disk1InUse, &cpuInUse);
					break;
				case Event::PROCESS_FINISH_DISK2:
					handle_process_finish_disk2(handledEvent, &priorityQueue, &disk2Queue, &cpuQueue, &disk2InUse, &cpuInUse);
					break;
				case Event::SIMULATION_FINISH:
					handle_simulation_finish(handledEvent, &priorityQueue);
					break;
				}
			}
		}
	}

	return 0;
}

void handle_process_arrival(Event event, std::vector<Event> *priorityQueue, std::vector<Process> *cpuQueue, bool *cpuInUse)
{
	// fetches interval and creates events relative to the status of the cpu
	int deleted = 0; // used to avoid deleting more than one event on the priority queue
	std::cout << "PROCESS HAS ARRIVED\n";
	::gTime = event.getEventTime();
	int arriveInterval = get_random_interval();
	std::cout << "ARRIVAL INTERVAL: " << arriveInterval << std::endl;
	if (*cpuInUse == true || cpuQueue->size() > 0)
	{
		int time = get_queue_time(priorityQueue, 1);
		Event ev = Event(::gTime + time, Event::PROCESS_ARRIVE_CPU, event.getProcess());
		rearrange_priority_queue(priorityQueue, ev);
		*cpuQueue->insert(cpuQueue->begin(), event.getProcess());
	}
	else if (*cpuInUse == false)
	{
		Event ev = Event(::gTime, Event::PROCESS_ARRIVE_CPU, event.getProcess());
		deleted = 1;
		rearrange_priority_queue(priorityQueue, ev);
		*cpuInUse = true;
	}

	// creates new process and attaches it to an event
	Process proc = Process();
	proc.setProcessID(::gpId);
	proc.setProcessState(Process::IN_QUEUE);
	Event ev = Event(::gTime + arriveInterval, Event::PROCESS_ARRIVAL, proc);
	if (deleted == 0)
	{
		priorityQueue->pop_back();
	}
	rearrange_priority_queue(priorityQueue, ev);
	::gProcessNum += 1;

	priorityQueue->pop_back();

	print_priority_queue(priorityQueue);
	::gpId += 1;
}

void handle_process_cpu_arrival(Event event, std::vector<Event>* priorityQueue, std::vector<Process> *cpuQueue, bool *cpuInUse)
{
	// creates 
	::gTime = event.getEventTime();
	if (cpuQueue->size() > 0)
	{
		cpuQueue->pop_back();
		*cpuInUse = true;
		std::cout << "PROCESS HAS ARRIVED AT THE CPU AT " << ::gTime << std::endl;
		int interval = get_cpu_interval();
		Event ev = Event(::gTime + interval, Event::PROCESS_FINISH_CPU, event.getProcess());
		rearrange_priority_queue(priorityQueue, ev);
	}
	else
	{
		*cpuInUse = true;
		std::cout << "PROCESS HAS ARRIVED AT THE CPU AT " << ::gTime << std::endl;
		int interval = get_cpu_interval();
		Event ev = Event(::gTime + interval, Event::PROCESS_FINISH_CPU, event.getProcess());
		rearrange_priority_queue(priorityQueue, ev);
	}
	
	priorityQueue->pop_back();
	print_priority_queue(priorityQueue);
}

void handle_process_exit_cpu(Event event, std::vector<Event> *priorityQueue, std::vector<Process> *cpuQueue, std::vector<Process> *disk1Queue, std::vector<Process> *disk2Queue, bool *cpuInUse, bool *disk1InUse, bool *disk2InUse)
{
	std::cout << "PROCESS HAS EXITED THE CPU AT " << ::gTime << std::endl;
	::gTime = event.getEventTime();
	bool prob = get_exit_probability();
	*cpuInUse = false;

	// determines if the process will exit or not, and then creates respective events relative to the status of the two disks and the size of their queues
	if (prob == true)
	{
		std::cout << "PROB IS TRUE IN CPU_EXIT.\n";
		Event ev = Event(::gTime, Event::PROCESS_EXIT_SYSTEM, event.getProcess());
		rearrange_priority_queue(priorityQueue, ev);
	}
	else if (prob == false)
	{
		std::cout << "PROB IS FALSE IN CPU_EXIT.\n";
		if (*disk1InUse == false && disk1Queue->size() == 0)
		{
			std::cout << "DISK 1 NOT IN USE";
			Event ev = Event(::gTime, Event::PROCESS_ARRIVE_DISK1, event.getProcess());
			rearrange_priority_queue(priorityQueue, ev);
			*disk1InUse = true;
		}
		else if (*disk1InUse == true && *disk2InUse == false && disk2Queue->size() == 0)
		{
			Event ev = Event(::gTime, Event::PROCESS_ARRIVE_DISK2, event.getProcess());
			rearrange_priority_queue(priorityQueue, ev);
			*disk2InUse = true;
		}
		else if ((*disk1InUse == true && *disk2InUse == true) && (disk1Queue->size() > 0 && disk2Queue->size() > 0))
		{
			if (disk1Queue->size() > disk2Queue->size())
			{
				int time = get_queue_time(priorityQueue, 2);
				Event ev = Event(::gTime + time, Event::PROCESS_ARRIVE_DISK2, event.getProcess());
				rearrange_priority_queue(priorityQueue, ev);
				*disk2Queue->insert(disk2Queue->begin(), event.getProcess());
			}
			else if (disk1Queue->size() <= disk2Queue->size())
			{
				int time = get_queue_time(priorityQueue, 2);
				Event ev = Event(::gTime + time, Event::PROCESS_ARRIVE_DISK1, event.getProcess());
				rearrange_priority_queue(priorityQueue, ev);
				*disk1Queue->insert(disk1Queue->begin(), event.getProcess());
			}
		}
	} 
	if (cpuQueue->size() > 0)
	{
		Event ev = Event(::gTime, Event::PROCESS_ARRIVE_CPU, cpuQueue->back());
		rearrange_priority_queue(priorityQueue, ev);
		*cpuInUse = true;
	}
	priorityQueue->pop_back();
	print_priority_queue(priorityQueue);
}

// deletes process if the exit event is triggered
void handle_process_exit_system(Event event, std::vector<Event> *priorityQueue, std::vector<Process> *cpuQueue)
{
	std::cout << "PROCESS HAS EXITED THE SYSTEM AT " << ::gTime << std::endl;
	event.getProcess().~Process();
	print_priority_queue(priorityQueue);
	priorityQueue->pop_back();
	::gProcessNum -= 1;
}

// creates event for finishing on disk 1
void handle_process_arrive_disk1(Event event, std::vector<Event> *priorityQueue, std::vector<Process> *disk1Queue, bool *disk1InUse)
{
	std::cout << "PROCESS HAS ARRIVED AT DISK 1 AT " << ::gTime << std::endl;
	::gTime = event.getEventTime();
	int time = get_disk_time();
	*disk1InUse = true;
	Event ev = Event(::gTime + time, Event::PROCESS_FINISH_DISK1, event.getProcess());
	rearrange_priority_queue(priorityQueue, ev);
	priorityQueue->pop_back();
	print_priority_queue(priorityQueue);
}

// creates event for finishing on disk 2
void handle_process_arrive_disk2(Event event, std::vector<Event> *priorityQueue, std::vector<Process> *disk2Queue, bool *disk2InUse)
{
	std::cout << "PROCESS HAS ARRIVED AT DISK 2 AT " << ::gTime << std::endl;
	::gTime = event.getEventTime();
	int time = get_disk_time();
	*disk2InUse = true;
	Event ev = Event(::gTime + time, Event::PROCESS_FINISH_DISK2, event.getProcess());
	rearrange_priority_queue(priorityQueue, ev);
	priorityQueue->pop_back();
	print_priority_queue(priorityQueue);
}

void handle_process_finish_disk1(Event event, std::vector<Event> *priorityQueue, std::vector<Process> *disk1Queue, std::vector<Process> *cpuQueue, bool *disk1InUse, bool *cpuInUse)
{
	std::cout << "PROCESS HAS FINISHED AT DISK 1 AT " << ::gTime << std::endl;
	::gTime = event.getEventTime();
	*disk1InUse = false;
	// creates different events by the status of the CPU and disk 1 queue size
	if (*cpuInUse == false && cpuQueue->size() == 0)
	{
		Event ev = Event(::gTime, Event::PROCESS_ARRIVE_CPU, event.getProcess());
		rearrange_priority_queue(priorityQueue, ev);
		priorityQueue->pop_back();
		*cpuInUse = true;
	}
	else if (*cpuInUse == true || cpuQueue->size() > 0)
	{
		int time = get_queue_time(priorityQueue, 1);
		Event ev = Event(::gTime + time, Event::PROCESS_ARRIVE_CPU, event.getProcess());
		rearrange_priority_queue(priorityQueue, ev);
		priorityQueue->pop_back();
		*cpuQueue->insert(cpuQueue->begin(), event.getProcess());
	}
	if (disk1Queue->size() > 0)
	{
		int time = get_disk_time();
		Event ev = Event(::gTime, Event::PROCESS_ARRIVE_DISK1, disk1Queue->back());
		rearrange_priority_queue(priorityQueue, ev);
		priorityQueue->pop_back();
		*disk1InUse = true;
		disk1Queue->pop_back();
	}

	print_priority_queue(priorityQueue);
}

void handle_process_finish_disk2(Event event, std::vector<Event> *priorityQueue, std::vector<Process> *disk2Queue, std::vector<Process> *cpuQueue, bool *disk2InUse, bool *cpuInUse)
{
	std::cout << "PROCESS HAS FINISHED AT DISK 2 AT " << ::gTime << std::endl;
	::gTime = event.getEventTime();
	*disk2InUse = false;
	// creates different events by the status of the CPU and disk 2 queue size
	if (*cpuInUse == false && cpuQueue->size() == 0)
	{
		Event ev = Event(::gTime, Event::PROCESS_ARRIVE_CPU, event.getProcess());
		rearrange_priority_queue(priorityQueue, ev);
		priorityQueue->pop_back();
		*cpuInUse = true;
	}
	else if (*cpuInUse == true || cpuQueue->size() > 0)
	{
		int time = get_queue_time(priorityQueue, 1);
		Event ev = Event(::gTime + time, Event::PROCESS_ARRIVE_CPU, event.getProcess());
		rearrange_priority_queue(priorityQueue, ev);
		priorityQueue->pop_back();
		*cpuQueue->insert(cpuQueue->begin(), event.getProcess());
	}
	if (disk2Queue->size() > 0)
	{
		int time = get_disk_time(); 
		Event ev = Event(::gTime, Event::PROCESS_ARRIVE_DISK2, disk2Queue->back());
		rearrange_priority_queue(priorityQueue, ev);
		priorityQueue->pop_back();
		*disk2InUse = true;
		disk2Queue->pop_back();
	}
	
	print_priority_queue(priorityQueue);
}

// deletes all other events in priority queue that have times passed the simulation time
void handle_simulation_finish(Event event, std::vector<Event> *priorityQueue)
{
	::gTime = event.getEventTime();
	for (unsigned int i = 0; i < priorityQueue->size(); i++)
	{
		priorityQueue->pop_back();
	}
	std::cout << "******SIMULATION HAS BEEN TERMINATED******\n";
}

// determines the disk interval
int get_disk_time()
{
	int min = 0;
	int max = 0;
	int time = 0;
	std::ifstream config;
	std::string line;

	config.open("CONFIG.txt");

	while (std::getline(config, line))
	{
		int fileTime;
		std::istringstream iss(line);
		std::string header;
		iss >> header >> fileTime;

		if (header == "DISK_MIN")
		{
			min = fileTime;
		}

		else if (header == "DISK_MAX")
		{
			max = fileTime;
		}
	}

	config.close();

	time = ::gTime + (rand() % (max - min + 1) + min);
	return time;
}

// determines arrival interval
int get_random_interval()
{
	int min = 0;
	int max = 0;
	int interval = 0;
	std::ifstream config;
	std::string line;

	config.open("CONFIG.txt");

	while (std::getline(config, line))
	{
		int fileInterval;
		std::istringstream iss(line);
		std::string header;
		iss >> header >> fileInterval;

		if (header == "ARRIVE_MIN")
		{
			min = fileInterval;
		}

		else if (header == "ARRIVE_MAX")
		{
			max = fileInterval;
		}
	}

	config.close();

	interval = rand() % (max - min + 1) + min;
	return interval;
}

/* This function unfortunately proved to be the most difficult.
   I implemented a version of this that allowed the program to advance
   through the simulation passed the initial point, but it produced many bugs.
   In an effort to improve this, I attempted to use a bubble sort by event time
   to first sort the priority queue by time, but was unable to find an adequate 
   solution in time. 
*/
void rearrange_priority_queue(std::vector<Event> *priorityQueue, Event event)
{
	// bubble sort to organize the queue
	int min = priorityQueue->at(0).getEventTime();
	
	if (priorityQueue->size() >= 2)
	{
		for (unsigned int i = 0; i < priorityQueue->size() - 1; i++)
		{
			for (unsigned int j = 0; j < priorityQueue->size() - i - 1; j++)
			{
				if (priorityQueue->at(j).getEventTime() < priorityQueue->at(j + 1).getEventTime())
				{
					swap(&priorityQueue->at(j), &priorityQueue->at(j + 1));
				}
			}
		}
	} // end sort
	else
	{
		priorityQueue->insert(priorityQueue->begin(), event);
	}

	// unsucessful attempt to insert the new event in its proper place, sorted by time
	for (unsigned int k = 0; k < priorityQueue->size() - 1; k++)
	{
		if (priorityQueue->at(k).getEventTime() > priorityQueue->at(k + 1).getEventTime())
		{
			priorityQueue->insert(priorityQueue->begin() + k, event);
		}
	}

}

// fetch for the exit probability
bool get_exit_probability()
{
	float min = 0;
	float max = 1;
	float prob = 0;
	std::ifstream config;
	std::string line;

	config.open("CONFIG.txt");

	while (std::getline(config, line))
	{
		float fileProb;
		std::istringstream iss(line);
		std::string header;
		iss >> header >> fileProb;

		if (header == "QUIT_PROB")
		{
			min = fileProb;
		}

	}

	config.close();
	prob = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	std::cout << "QUIT PROB IS " << prob << std::endl;
	std::cout << "THIS MEANS IT'S " << (prob <= min ? 1 : 0) << std::endl;
	if (prob <= min)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// fetch for cpu time interval
int get_cpu_interval()
{
	int interval = 0;
	int min = 0;
	int max = 0;
	std::ifstream config;
	std::string line;

	config.open("CONFIG.txt");

	while (std::getline(config, line))
	{
		int fileCpuTime;
		std::istringstream iss(line);
		std::string header;
		iss >> header >> fileCpuTime;

		if (header == "CPU_MIN")
		{
			min = fileCpuTime;
		}

		else if (header == "CPU_MAX")
		{
			max = fileCpuTime;
		}
	}
	
	config.close();
	interval = rand() % ((max - min + 1) + min);
	return interval;
}

// used for testing purposes
void print_priority_queue(std::vector<Event> *priorityQueue)
{
	std::cout << "PRIORITY QUEUE SIZE: " << priorityQueue->size() << std::endl;
	for (unsigned int i = 0; i < priorityQueue->size(); i++)
	{
		std::cout << "=================================================================\n";
		std::cout << "TYPE: " << priorityQueue->at(i).getEventType() << std::endl;
		std::cout << "PROCESS LINKED TO IT: " << priorityQueue->at(i).getProcess().getProcessID() << std::endl;
		std::cout << "EVENT TIME: " << priorityQueue->at(i).getEventTime() << std::endl;
		std::cout << "EVENT AT THE END OF QUEUE HAS " << priorityQueue->back().getEventTime() << " TIME.\n";
		std::cout << "=================================================================\n";
	}
}

// used to fetch time of other events in the different queues to set the appropriate time for the next event to be serviced
int get_queue_time(std::vector<Event> *priorityQueue, int type)
{
	int time = 0;
	switch (type) {
	case 1:
		for (unsigned int i = 0; i < priorityQueue->size(); i++)
		{
			if (priorityQueue->at(i).getEventType() == Event::PROCESS_ARRIVE_CPU)
			{
				time += priorityQueue->at(i).getEventTime();
			}
		}
		break;
	case 2:
		for (unsigned int i = 0; i < priorityQueue->size(); i++)
		{
			if (priorityQueue->at(i).getEventType() == Event::PROCESS_ARRIVE_DISK1 || priorityQueue->at(i).getEventType() == Event::PROCESS_ARRIVE_DISK2)
			{
				time += priorityQueue->at(i).getEventTime();
			}
		}
	}
	return time;
}

// checks simulation time to close
bool check_close()
{
	bool close = false;
	int closeTime = 0;
	std::ifstream config;
	std::string line;

	config.open("CONFIG.txt");

	while (std::getline(config, line))
	{
		int check;
		std::istringstream iss(line);
		std::string header;
		iss >> header >> check;

		if (header == "FIN_TIME")
		{
			closeTime = check;
		}
	}

	config.close();
	if (::gTime >= closeTime)
	{
		close = true;
	}
	else
	{
		close = false;
	}
	return close;
}

// swap function used in bubble sort
void swap(Event *x, Event *y)
{
	Event temp = *x;
	*x = *y;
	*y = temp;
}
