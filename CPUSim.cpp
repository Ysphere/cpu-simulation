#include "CPUSim.h"
#include <sstream>
#include <string>
#include <memory>
#include <string.h>

CPUSim::CPUSim()
{
	clock = 0;

	mode = NEWCPU;

	prev_process = -1;
	wait = 0;
	cpu_is_executing = 0;

	detailed = UNSET;
	verbose = UNSET;
	round_robin = UNSET;
	time_quantum = NO_QUANTUM_VALUE;

	total_cpu_execution_time = 0;
	thread_switch = -1;
	process_switch = -1;
	num_of_threads = -1;
	num_of_processes = -1;
}

void CPUSim::addFinishedIOThreadsToReadyQueue()
{
	std::shared_ptr<Thread> arriving_thread = nullptr;

	do
	{
		/*removes a thread from IO queue which has no IO time left*/
		arriving_thread = io_queue.removeIOThreadAtTime(IO_COMPLETED);

		/*if thread is NULL, that means there are none with that time, we skip this if*/
		if (arriving_thread != nullptr)
		{
			if (verbose == SET)
			{
				/*if in verbose mode, print verbose description*/
				std::cout << "At Time " << clock << ": Thread " << arriving_thread->getThreadNumber() << " of Process " << arriving_thread->getProcessNumber() << " moves from BLOCKED to READY" << std::endl;
			}

			/*take thread we removed from IO queue and add to Ready queue*/
			addThread(arriving_thread, READY);
		}
		/*if thread was null, nothing is done and loop is exited*/
	} while (arriving_thread != nullptr);
}

void CPUSim::addArrivingIOThreadsToReadyQueue()
{
	std::shared_ptr<Thread> arriving_thread = nullptr;

	do
	{
		/*try to remove threads from job_queue which have an arrival time
		equal to the current CPU clock time*/
		arriving_thread = job_queue.removeThreadAtTime(clock);

		/*if thread was not null, decrement size of job queue*/
		if (arriving_thread != nullptr)
		{
			/*if verbose print as so*/
			if (verbose == SET)
			{
				std::cout << "At Time " << clock << ": Thread " << arriving_thread->getThreadNumber() << " of Process " << arriving_thread->getProcessNumber() << " moves from NEW to READY" << std::endl;
			}

			/*add arriving thread to ready queue*/
			addThread(arriving_thread, READY);
		}
	} while (arriving_thread != nullptr);
}

void CPUSim::addThread(std::shared_ptr<Thread> thread, Destination dest)
{
	/*mode enum signifies which queue to add to*/
	if (dest == READY)
	{
		ready_queue.addThread(thread);
	}

	if (dest == IO)
	{
		io_queue.addThread(thread);
	}

	if (dest == JOB)
	{
		job_queue.addThread(thread);
	}
}

bool CPUSim::canContinue(SimQueue & exit_queue)
{
	if (num_of_threads != exit_queue.size())
	{
		return true;
	}
	else
	{
		return false;
	}
}

int CPUSim::executeThreadFCFS(SimQueue & q)
{
	/*EXECUTING can mean either start a new burst or continue on an old one*/
	/*if cpu_is_executing = 0, start executing a new procoess*/
	if (cpu_is_executing == 0)
	{
		/*current thread set beforehand*/
		/*set timings sets the length of the CPU and IO bursts to be executed right now*/
		current_thread->setTimings();
		/*set the CPU wait to the length of the cpu burst*/
		wait = current_thread->getCPUTime(); /*setting cpu to wait for length of cpu burst (ie do not execute any more threads)*/

											 /*if this is the first burst in the thread, we set the start time of the thread*/
		if (current_thread->getStartTime() == -1)
		{
			current_thread->setStartTime(clock); /*setting start time to current time*/
		}

		/*if we are not on last burst pair, add the IO time to the threads total*/
		if (current_thread->getIOTimeRemaining() >= 0)
		{
			/*add the current IO burst to the total IO done by the thread so far*/
			current_thread->setIOThreadTotal(current_thread->getIOTimeRemaining() + current_thread->getIOThreadTotal());
		}

		/*verbose print*/
		if (verbose == SET)
		{
			std::cout << "At Time " << clock << ": Thread " << current_thread->getThreadNumber() << " of Process " << current_thread->getProcessNumber() << " moves from READY to RUNNING" << std::endl;
		}

		/*the cpu is now executing a burst so we chaning the cpu_is_executing to reflect that*/
		cpu_is_executing = 1;
	}
	/*if the CPU is in the middle of a burst*/
	else if (cpu_is_executing == 1)
	{
		/*decrement the wait (every tick passes is one closer to being done the burst)*/
		wait--;
		/*increase the total amount of cpu execution time*/
		total_cpu_execution_time++;
		/*increse the total cpu time of the thread*/
		current_thread->cpuThreadTotalIncrease(1);

		/*if wait is done, eg we can move this thread out and start on a new one*/
		if (wait == 1)
		{
			/*if the io time of the burst is -1, we know that was the last CPU burst
			so we move the current_thread to EXIT*/
			if (current_thread->getIOTimeRemaining() == -1)
			{
				/*set exit time of the thread*/
				current_thread->setExitTime(clock);

				/*add to the queue that holds all exited threads (passed to this function)*/
				q.addThread(current_thread);

				/*verbose print*/
				if (verbose == SET)
				{
					std::cout << "At Time " << clock << ": Thread " << current_thread->getThreadNumber() << " of Process " << current_thread->getProcessNumber() << " moves from RUNNING to EXIT" << std::endl;
				}
			}
			else
			{
				/*if not exiting, move the thread to the IO queue so it can do its IO time*/
				if (verbose == SET)
				{
					std::cout << "At Time " << clock << ": Thread " << current_thread->getThreadNumber() << " of Process " << current_thread->getProcessNumber() << " moves from RUNNING to BLOCKED" << std::endl;
				}

				/*add thread to io_queue*/
				addThread(current_thread, IO);
			}

			/*after moving the current thread out of cpu, we need a new one so we set the
			cpu mode back to dispatching to get a new one*/
			setMode(DISPATCHING);
			/*cpu is no longer executing*/
			cpu_is_executing = 0;
		}
	}
	return 1;
}

int	CPUSim::executeThreadRR(SimQueue & q)
{
	/*EXECUTING can mean either start a new burst or continue on an old one*/
	/*if cpu_is_executing = 0, start executing a new procoess*/
	if (cpu_is_executing == 0)
	{
		/*current thread set beforehand*/
		/*set timings sets the length of the CPU and IO bursts to be executed right now*/
		current_thread->setTimings();
		/*set the CPU wait to the length of the time quantum*/
		wait = time_quantum;

		/*if this is the first burst in the thread, we set the start time of the thread*/
		if (current_thread->getStartTime() == -1)
		{
			current_thread->setStartTime(clock); /*setting start time to current time*/
		}

		/*if we are not on last burst pair, add the IO time to the threads total*/
		if (current_thread->getIOTimeRemaining() >= 0)
		{
			/*add the current IO burst to the total IO done by the thread so far*/
			current_thread->setIOThreadTotal(current_thread->getIOTimeRemaining() + current_thread->getIOThreadTotal());
		}

		/*verbose print*/
		if (verbose == SET)
		{
			std::cout << "At Time " << clock << ": Thread " << current_thread->getThreadNumber() << " of Process " << current_thread->getProcessNumber() << " moves from READY to RUNNING" << std::endl;
		}

		/*the cpu is now executing a burst so we chaning the cpu_is_executing to reflect that*/
		cpu_is_executing = 1;
	}
	/*if the CPU is in the middle of a burst*/
	else if (cpu_is_executing == 1)
	{
		/*decrement the wait (every tick passes is one closer to being done the burst)*/
		wait--;
		/*decrement cpu_time as well, because end of time slice, or end of burst means switch*/
		current_thread->cpuTimeIncrease(-1);
		/*increase the total amount of cpu execution time*/
		total_cpu_execution_time++;
		/*increse the total cpu time of the thread*/
		current_thread->cpuThreadTotalIncrease(1);

		/*if wait (time slice) is done or burst is done,
		we can move this thread out and start on a new one*/
		if (wait == 1 || current_thread->getCPUTime() == 1)
		{
			/*if the io time of the burst is -1, we know that was the last CPU burst
			so we move the current_thread to EXIT*/
			if (current_thread->getIOTimeRemaining() == -1)
			{
				/*set exit time of the thread*/
				current_thread->setExitTime(clock);

				/*add to the queue that holds all exited threads (passed to this function)*/
				q.addThread(current_thread);

				/*verbose print*/
				if (verbose == SET)
				{
					std::cout << "At Time " << clock << ": Thread " << current_thread->getThreadNumber() << " of Process " << current_thread->getProcessNumber() << " moves from RUNNING to EXIT" << std::endl;
				}
			}
			/*add the rest of the burst back to the execution stack*/
			else if (wait == 1 && current_thread->getCPUTime() != 1)
			{
				current_thread->addBurst(current_thread->getCPUTime(), current_thread->getIOTimeRemaining());
				/*if not exiting, move the thread to the IO queue so it can do its IO time*/
				if (verbose == SET)
				{
					std::cout << "At Time " << clock << ": Thread " << current_thread->getThreadNumber() << " of Process " << current_thread->getProcessNumber() << " moves from RUNNING to READY" << std::endl;
				}

				/*add thread to io_queue*/
				addThread(current_thread, READY);
			}
			else
			{
				/*if not exiting, move the thread to the IO queue so it can do its IO time*/
				if (verbose == SET)
				{
					std::cout << "At Time " << clock << ": Thread " << current_thread->getThreadNumber() << " of Process " << current_thread->getProcessNumber() << " moves from RUNNING to BLOCKED" << std::endl;
				}


				/*add thread to io_queue*/
				addThread(current_thread, IO);
			}

			/*after moving the current thread out of cpu, we need a new one so we set the
			cpu mode back to dispatching to get a new one*/
			setMode(DISPATCHING);
			/*cpu is no longer executing*/
			cpu_is_executing = 0;
		}
	}
	return 1;
}

int CPUSim::getNumberOfProcesses()
{
	return num_of_processes;
}

int CPUSim::getNumberOfThreads()
{

	return num_of_threads;
}

int CPUSim::getNextThread()
{
	std::shared_ptr<Thread>  next_thread = nullptr;

	/*grab thread from ready queue*/
	next_thread = ready_queue.removeThread();

	if (next_thread != nullptr)
	{
		/*sets the current thread of the cpu to the thread pulled from the ready queue
		this thread will be used once the cpu goes into EXECUTING mode*/
		current_thread = next_thread;

		/*if the previous thread was from the same process, we do a thread switch*/
		if (prev_process == current_thread->getProcessNumber())
		{
			/*cpu goes into thread switch mode*/
			setMode(TSWITCH);
		}
		else /*if not from the same process, we do a process switch*/
		{
			/*set the new previous process*/
			prev_process = current_thread->getProcessNumber();
			/*change cpu to process switch mode*/
			setMode(PSWITCH);
		}
	}

	return 1;
}

void CPUSim::advanceClock()
{
	clock++;

	io_queue.decrementAllIO();
}

void CPUSim::calculateStatistics(SimQueue & q)
{
	clock--; /*one extra clock tick upon exit, so removing it here*/

			 /*if detailed is set, get detailed statistics*/
	if (detailed == SET)
	{
		stats_detailed(*this, q);
	}
	else
	{
		stats_default(*this, q);
	}

}

void CPUSim::checkStatus()
{
	/*decrement the wait*/
	wait--;

	/*if wait is over...*/
	if (wait == 0)
	{
		/*move cpu into executing mode*/
		setMode(EXECUTING);
		wait = 0;
	}
}

void CPUSim::executeThread(SimQueue & q)
{
	if (round_robin = SET)
	{
		executeThreadRR(q);
	}
	else
	{
		executeThreadFCFS(q);
	}
}

void CPUSim::setMode(Mode mode)
{
	if (mode == TSWITCH)
	{
		/*if we are switching into threadswitch mode, the cpu wait is set to
		the length of thread switch parsed from file*/
		wait = thread_switch;
	}
	else if (mode == PSWITCH)
	{
		/*same for process switch*/
		wait = process_switch;
	}

	/*set the mode now*/
	this->mode = mode;
}


void stats_default(CPUSim & cpu, SimQueue q)
{
	float cpu_util = 0;
	int total_time = cpu.clock;

	/*calculate cpu utilization*/
	cpu_util = ((float)cpu.total_cpu_execution_time / cpu.clock) * 100;

	/*pass to print function*/
	printDefaultStats(cpu, q, cpu_util, total_time);
}

/*prints the final stats of the CPUSim in detailed mode, threads presented in exit order*/
void stats_detailed(CPUSim & cpu, SimQueue q)
{
	/*default stats are part of the detailed stats*/
	stats_default(cpu, q);

	/*temp thread points to head of the exit_queue which contains all exited threads*/

	for (auto p : q.q)
	{
		/*print detailed info*/
		printf("\n");
		printf("Thread %d of Process %d:\n\n", p->getThreadNumber(), p->getProcessNumber());
		printf("arrival time: %d\n", p->getArrivalTime());
		printf("service time: %d\n", p->getCPUThreadTotal());
		printf("I/O time: %d\n", p->getIOThreadTotal());
		printf("turnaround time: %d\n", p->getExitTime() - p->getArrivalTime());
		printf("exit time: %d\n", p->getExitTime());
		printf("\n");
	}
}

/*printing function*/
void printDefaultStats(CPUSim & cpu, SimQueue q, float cpu_util, int time)
{
	/*chooses which mode to print out based on what mode the cpu executed in*/
	if (cpu.time_quantum != -1)
	{
		printf("\nRound Robin (with time quantum = %d): \n\n", cpu.time_quantum);
	}
	else
	{
		printf("\nFCFS:\n\n");
	}

	/*printing default stats*/
	printf("Total Time required is %d time units\n", time);
	printf("Average Turnaround Time is %.1f time units\n", turnaroundTime(cpu, q));
	printf("CPU Utilization is %.0f percent\n\n", cpu_util);
}

float turnaroundTime(CPUSim & cpu, SimQueue q)
{
	float arr_temp = 0;
	float exit_temp = 0;
	float turnaround = 0;
	std::shared_ptr<Thread> temp = nullptr;

	for (int i = 1; i <= cpu.num_of_processes; i++)
	{
		arr_temp = exit_temp = 0;

		for (auto p : q.q)
		{
			if (p->getProcessNumber() == i)
			{
				arr_temp = p->getArrivalTime();
				exit_temp = p->getExitTime();
			}
		}

		turnaround += (exit_temp - arr_temp);
	}

	return turnaround / (cpu.num_of_processes);
}

int initializeJobQueue(CPUSim & cpu)
{

	parseCPUInfo(cpu); /*parses the first line of the file bc it does not show up in the file pattern again*/

	parseProcesses(cpu); /*parse all processes in the file, based off of info from parseCPUInfo*/

						 /*after all processes are parsed, set the number of threads the cpu has, to the size
						 of the job queue*/
	cpu.num_of_threads = cpu.job_queue.size();

	return 1;
}

/*responsible for parsing all processes and their threads in file*/
int parseProcesses(CPUSim & cpu)
{
	int process_num = 0;

	/*for the number of processes in the file...*/
	for (int i = 0; i < cpu.num_of_processes; i++)
	{
		/*scan in process number, and num of threads in said process*/
		std::cin.ignore(200, '\n');
		std::cin >> process_num >> cpu.num_of_threads;
		/*parse threads based off number of threads in process*/
		parseThreads(cpu, process_num);
	}

	return 1;
}

/*responsible for parsing all threads in a given process*/
int parseThreads(CPUSim & cpu, int process_num)
{
	/*for all threads in a process...*/
	for (int i = 0; i < cpu.getNumberOfThreads(); i++)
	{
		/*parse a thread*/
		parseThread(cpu, process_num);
	}

	return 1;
}

/*parses one thread from file*/
int parseThread(CPUSim & cpu, int process_num)
{
	int thread_number = 0;
	int arrival_time = -1;
	int num_of_bursts = 0;
	std::shared_ptr<Thread> new_thread = NULL;

	/*scan in thread num wrt to process, its arrival time, and number of cpu burts*/
	std::cin.ignore(200, '\n');
	std::cin >> thread_number >> arrival_time >> num_of_bursts;

	/*create a new thread object with parsed info*/
	new_thread = std::make_shared<Thread>(process_num, thread_number, arrival_time, num_of_bursts);

	/*parse the execution stack of the thread based on 'num_of_bursts'*/
	parseBursts(new_thread);

	/*add the thread to the job queue inside our cpu sim*/
	cpu.addThread(new_thread, JOB);

	return 1;
}

/*parses the execution stack of one thread*/
int parseBursts(std::shared_ptr<Thread> thread)
{
	int burst_num = 0;
	int cpu_time = 0;
	int io_time = 0;

	if (!thread)
	{
		return -1;
	}

	/*for x-1 number of bursts in the stack...*/
	for (int i = 0; i < (thread->getNumberOfBursts() - 1); i++)
	{
		/*scan in info*/
		std::cin.ignore(200, '\n');
		std::cin >> burst_num >> cpu_time >> io_time;
		/*add info to execution stack object (BurstQueue) inside the already created thread*/
		thread->addBurst(cpu_time, io_time);
	}

	/*scan in the last burst seperatley, because we expect the last burst to have no io*/
	std::cin.ignore(200, '\n');
	std::cin >> burst_num >> cpu_time;
	/*set last io burst to -1 for signal use later*/
	thread->addBurst(cpu_time, -1);

	return 1;
}

/*parses the first line of the file*/
int parseCPUInfo(CPUSim & cpu)
{
	std::cin >> cpu.num_of_processes >> cpu.thread_switch >> cpu.process_switch;

	return 1;
}

/*responsible for setting flags inside CPUSim object to set output style, scheduling etc...*/
void processCommandLineArgs(CPUSim & cpu, char ** argv, int argc)
{
	/*make sure there are not too many arguements on the cmd line, exit if there are too many*/
	if (argc > 7)
	{
		printf("Invalid command line parameters. Exiting.\n");
		exit(0);
	}

	/*for all args in argv...*/
	for (int i = 0; i < argc; i++)
	{
		/*compare arg with flag*/
		if (strcmp(argv[i], "-d") == 0)
		{
			/*if flag recognized, set corresponding flag in CPUSim object*/
			cpu.detailed = SET;
			break;
		}
	}

	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "-v") == 0)
		{
			cpu.verbose = SET;
			break;
		}
	}

	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "-r") == 0)
		{
			cpu.round_robin = SET;
			break;
		}
	}

	/*for all args in argv...*/
	for (int i = 0; i < argc; i++)
	{
		/*check the first letter to see if it is a digit*/
		if (isdigit(argv[i][0]))
		{
			/*if it is, turn that args into an integer, and set time quantum in CPUSim*/
			int num = atoi(argv[i]);
			cpu.time_quantum = num;
			break;
		}
	}
}
