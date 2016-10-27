/* Main function to kick off the Squirrel disease simulation
   The master process will call simMasterCode()
   The worker processes will call simWorkerCode()
   Abstracted away the implementation details of these in the simulation specific code
*/

#include <stdio.h>
#include <mpi.h>
#include "processpool/pool.h"
#include "squirrelsim/runsqsim.h"

#define BUFFSIZE 50000

int main(int argc, char* argv[]) {
	//Initialize MPI
	MPI_Init(&argc, &argv);

	//Initialize the process pool
	int statusCode = processPoolInit();

	int buffer[BUFFSIZE];
	MPI_Buffer_attach( buffer, BUFFSIZE); 
	if(statusCode == 1) {
		simWorkerCode();
	} else if (statusCode ==2) {
		//Master process runs here on process of rank =0
		simMasterCode();
		//Master will poll for new workers spawning or die-ing 
		//it will also end simulation when it receives the shutdown call
		int masterstatus = masterPoll();
		while (masterstatus) {
			masterstatus = masterPoll();
		}
	}
	processPoolFinalise();
	MPI_Finalize();
	return 0;
}
