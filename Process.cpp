#pragma once
#include "Process.h"

int Process::getProcessID()
{
	return _processID;
}

void Process::setProcessID(int id)
{
	_processID = id;
}

Process::State Process::getProcessState()
{
	return _state;
}

void Process::setProcessState(State state)
{
	_state = state;
}
