#pragma once

class Process
{
public:
	enum State { IN_QUEUE, EXECUTING_ON_CPU, IO_ON_DISK };
	int getProcessID();
	void setProcessID(int);
	State getProcessState();
	void setProcessState(State);
	~Process() {}

private:
	int _processID;
	State _state;
};
