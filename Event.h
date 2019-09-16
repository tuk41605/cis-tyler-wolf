#include "Process.h"

class Event
{
public:
	enum Type {
		PROCESS_ARRIVAL, PROCESS_ARRIVE_CPU, PROCESS_FINISH_CPU, PROCESS_EXIT_SYSTEM,
		PROCESS_ARRIVE_DISK1, PROCESS_ARRIVE_DISK2, PROCESS_FINISH_DISK1, PROCESS_FINISH_DISK2, SIMULATION_FINISH
	};
	int getEventTime();
	void setEventTime(int);
	Type getEventType();
	void setEventType(Type);
	Process getProcess();
	Event(int, Type, Process);

private:
	int _time;
	Type _thisType;
	Process _process;
};
