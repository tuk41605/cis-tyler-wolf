#include "Event.h" 
#include "Process.h"

Event::Event(int time, Type type, Process process)
{
	_time = time;
	_thisType = type;
	_process = process;
}

int Event::getEventTime()
{
	return _time;
}

void Event::setEventTime(int time)
{
	_time = time;
}

Event::Type Event::getEventType()
{
	return _thisType;
}

void Event::setEventType(Type type)
{
	_thisType = type;
}

Process Event::getProcess()
{
	return _process;
}
