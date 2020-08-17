#include <stdio.h>
#include <unistd.h> 
#include "CPUScaler.h"
#include "AsyncEnergyMonitorCSide.h"

/////////////////////// \\\\\\\\\\\\\\\\\\\\|
 ///  DO NOT DELETE WHAT IS IN THIS     \\\ |
 ///  COMMENT SECTION SAVE IT TO THE    \\\ |
 ///  JAVA THREAD TEST DRIVER FILE WHEN \\\ |
 ///  YOU GET THE OPPORTUNITY           \\\ |
/////////////////////// \\\\\\\\\\\\\\\\\\\\|

void run_cthread(int argc, const char* argv[])
{
	printf("hello world\n");

	pthread_t thread;	
	AsyncEnergyMonitor* collector = newAsyncEnergyMonitor(0, thread);
	start(collector);
	sleep(5);
	stop(collector);
	writeToFile(collector, (argc>1)?argv[1]:NULL );
	freeAsyncEnergyMonitor(collector);

}
/////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

int main(int argc, const char* argv[])
{
	ProfileInit();

	run_cthread(argc,argv);

	ProfileDealloc();
}
