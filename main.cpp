#include "CPUSim.h"

int main(int argc, char ** argv)
{
	//freopen("testcase3.txt", "r+", stdin);

	CPUSim cpu;
	SimQueue exit_queue;

	processCommandLineArgs(cpu, argv, argc); /*sets flags and/or time quantum*/

	initializeJobQueue(cpu);

	while (cpu.canContinue(exit_queue)) /*if there are still threads to be worked on continue*/
	{
		switch (cpu.mode)
		{
		case NEWCPU: /*CPU in NEWCPU upon initialization*/
			cpu.setMode(DISPATCHING);
			break;
		case DISPATCHING:
			/*if cpu dispatching, get the next thread to execute,
			this function auto switches to either PSWITCH or
			TSWITCH cpu mode based on the circumstanses*/
			cpu.getNextThread();
			break;
		case EXECUTING:
			/*executes a burst or loads in a new one if there is not one executing*/
			/*function auto switches to dispatching once a thread is done its burst*/
			cpu.executeThread(exit_queue);
			break;
		case PSWITCH:
		case TSWITCH:
			/*check to see when we can exit the context switch and begin executing*/
			cpu.checkStatus();
			break;
		default:
			printf("Fatal Error. Exiting\n");
			exit(0);
		}

		/*move any arriving threads into ready queue*/
		cpu.addArrivingIOThreadsToReadyQueue();
		/*move any finished IO threads to ready queue*/
		cpu.addFinishedIOThreadsToReadyQueue();

		/*clock tick*/
		cpu.advanceClock();
	}

	/*once all threads exit we calculate and display stats*/
	cpu.calculateStatistics(exit_queue);

	return 0;
}
