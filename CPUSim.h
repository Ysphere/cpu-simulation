#pragma once

#include "SimQueue.h"
#include <memory>

#define NO_QUANTUM_VALUE -1
#define IO_COMPLETED 0
#define EXIT -99

/*Flag type used to represent possible user flag inputs (-d, -v) within the CPUSim structure*/
typedef enum Flag {
	UNSET = 0,  /*flags can be SET or UNSET*/
	SET = 1
} Flag;

/*used to tell a dispatcher function which queue to place an object in*/
typedef enum Destination {
	READY = 0,          /*objects can be placed in the IO, JOB, or READY queues*/
	IO = 1,
	JOB = 2
}Destination;

/*represents what the CPU is currently doing*/
typedef enum Mode {
	EXECUTING = 0,      /*CPU can be in one of 5 modes, see simcpu.c for further info*/
	DISPATCHING = 1,
	NEWCPU = 2,
	PSWITCH = 3,
	TSWITCH = 4
}Mode;


class CPUSim
{
public:
	CPUSim();

	void addFinishedIOThreadsToReadyQueue();

	void addArrivingIOThreadsToReadyQueue();

	void addThread(std::shared_ptr<Thread> thread, Destination dest);

	bool canContinue(SimQueue & exit_queue);

	int executeThreadFCFS(SimQueue & q);

	int	executeThreadRR(SimQueue & q);

	int getNumberOfProcesses();

	int getNumberOfThreads();

	int getNextThread();

	void advanceClock();

	void calculateStatistics(SimQueue & q);

	void checkStatus();

	void executeThread(SimQueue & q);

	void setMode(Mode mode);
	
public:
	Flag verbose;               /*SET if -v included in program invokation*/
	Flag detailed;              /*SET if -d included in program invokation*/
	Flag round_robin;           /*SET if -r included in program invokation*/
	int clock;                  /*the main clock for the CPU*/
	int cpu_is_executing;       /*set to 1 when the CPU is in the middle of a burst, 0 otherwise*/
	int num_of_threads;         /*total number of threads in all processes in CPU*/
	int num_of_processes;       /*number of processes being worked on by CPU*/
	int prev_process;           /*process number of previous process, uses for choosing between thread or process switch*/
	int process_switch;         /*time it takes to switch process*/
	int thread_switch;          /*time it takes to switch to different thread in same procees*/
	int time_quantum;           /*time quantum for use in RR if included*/
	int total_cpu_execution_time;   /*incremented for every CPU tick in which it is executing*/
	int wait;                   /*tells the cpu for how long to wait during context switch/burst execution before changing mode*/
	Mode mode;                  /*current mode of the CPU*/
	std::shared_ptr<Thread> current_thread;    /*the thread that the CPU is currently working on*/
	SimQueue ready_queue; /*CPU ready queue*/
	SimQueue io_queue;    /*CPU io queue, home of blocked threads*/
	SimQueue job_queue;   /*all threads parsed from file are initialized into job queue*/
};

void stats_default(CPUSim & cpu, SimQueue q);

/*prints the final stats of the CPUSim in detailed mode, threads presented in exit order*/
void stats_detailed(CPUSim & cpu, SimQueue q);

/*printing function*/
void printDefaultStats(CPUSim & cpu, SimQueue q, float cpu_util, int time);

float turnaroundTime(CPUSim & cpu, SimQueue q);

int initializeJobQueue(CPUSim & cpu);

/*responsible for parsing all processes and their threads in file*/
int parseProcesses(CPUSim & cpu);

/*responsible for parsing all threads in a given process*/
int parseThreads(CPUSim & cpu, int process_num);

/*parses one thread from file*/
int parseThread(CPUSim & cpu, int process_num);

/*parses the execution stack of one thread*/
int parseBursts(std::shared_ptr<Thread> thread);

/*parses the first line of the file*/
int parseCPUInfo(CPUSim & cpu);

/*responsible for setting flags inside CPUSim object to set output style, scheduling etc...*/
void processCommandLineArgs(CPUSim & cpu, char ** argv, int argc);
