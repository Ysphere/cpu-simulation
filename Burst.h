#pragma once

#include <iostream>

/*the BurstNode type holds a pair of bursts (1 io, and 1 cpu) on the execution stack (BurstQueue)*/

class Burst
{
public:
	Burst(int cpu_t, int io_t)
	{
		cpu_time = cpu_t;
		io_time = io_t;
	}
	int get_cpu_time()
	{
		return cpu_time;
	}
	int get_io_time()
	{
		return io_time;
	}
	void display()
	{
		std::cout << "\tcpu: " << cpu_time << " \t io: " << io_time << " " << std::endl;
	}
private:
	int cpu_time;               /*holds the length of a cpu burst on the execution stack*/
	int io_time;                /*holds length of an io burst on the execution stack*/
};