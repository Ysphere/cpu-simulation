#pragma once

#include "Burst.h"
#include <list>
#include <stdio.h>
#define DEFAULT_EXIT_VALUE -1

/*the thread type is the unit that is passed around between
various queues in the CPUSim (job, io, ready)*/

class Thread
{
public:
	Thread(int process_num, int thread_num, int arrival_t, int cpu_bursts)
	{
		process_number = process_num;
		thread_number = thread_num;

		cpu_time = 0;
		cpu_thread_total = 0;
		io_thread_total = 0;
		io_time_remaining = 0;

		arrival_time = arrival_t;
		start_time = -1;
		exit_time = DEFAULT_EXIT_VALUE;

		bursts = cpu_bursts;
	}

	void addBurst(int cpu_time, int io_time)
	{
		Burst burst(cpu_time, io_time);
		burst_queue.push_back(burst);
	}

	int getNumberOfBursts()
	{
		return bursts;
	}

	int getArrivalTime()
	{
		return arrival_time;
	}

	int getIOTimeRemaining()
	{
		return io_time_remaining;
	}

	int getThreadNumber()
	{
		return thread_number;
	}

	int getProcessNumber()
	{
		return process_number;
	}

	int getStartTime()
	{
		return start_time;
	}

	void setStartTime(int t)
	{
		start_time = t;
	}

	int getIOThreadTotal()
	{
		return io_thread_total;
	}

	void setIOThreadTotal(int t)
	{
		io_thread_total = t;
	}

	int getCPUTime()
	{
		return cpu_time;
	}
	void cpuTimeIncrease(int d)
	{
		cpu_time += d;
	}

	void cpuThreadTotalIncrease(int d)
	{
		cpu_thread_total += d;
	}

	void setExitTime(int t)
	{
		exit_time = t;
	}

	void decrement()
	{
		io_time_remaining--;
	}

	void setTimings()
	{
		Burst burst = burst_queue.front();
		cpu_time = burst.get_cpu_time();
		io_time_remaining = burst.get_io_time();
		burst_queue.pop_front();
	}

	int getCPUThreadTotal()
	{
		return cpu_thread_total;
	}

	int getExitTime()
	{
		return exit_time;
	}

	void print()
	{
		std::cout << "IO Time remaining: " << io_time_remaining << std::endl;
		std::cout << "Process Number: " << process_number << std::endl;
		std::cout << "Thread Number: " << thread_number << std::endl;
		std::cout << "Arrival Time: " << arrival_time << std::endl;
		std::cout << "Start Time: " << start_time << std::endl;
		std::cout << "Exit Time: " << exit_time << std::endl;
		std::cout << "Total CPU time: " << cpu_thread_total << std::endl;
		std::cout << "Total IO time: " << io_thread_total << std::endl;
		std::cout << "CPU Bursts: " << bursts << std::endl;
		for (Burst burst : burst_queue)
		{
			burst.display();
		}
		printf("\n");
	}

private:
	int process_number;         /*represents the process to which this thread belongs*/
	int thread_number;          /*thread number w.r.t. process*/
	int arrival_time;           /*time it arrives in CPUSim */
	int start_time;             /*time when it begins execution */
	int io_time_remaining;      /*for decrementing in IO queue*/
	int cpu_time;               /*length of the current cpu burst*/
	int io_thread_total;        /*total io time done by thread*/
	int cpu_thread_total;       /*total cpu time done by thread*/
	int exit_time;              /*time it exits the CPUSim*/
	int bursts;                 /*number of cpu-io burst pairs*/
	std::list<Burst> burst_queue;   /*execution stack of the thread*/
};
