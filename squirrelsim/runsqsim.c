/* 
	Master of the Squirrel Disease Simulation
	This job has two functions to it, 
		simMasterCode to be executed by the Master which is running the process pool
		simWorkerCode to be executed by all the worker actors spawned by the master
	The philosophy here is that everyone is an actor and all actors share information 
	using standard message protocols which are specified in actor.c and message.c
	This job uses the process pool provided to run the pool on Rank 0
	The worker processes spawned will be on other ranks. 
	The simulation will not be run on Rank 0.
*/

#include <stdio.h>
#include <mpi.h>
#include "../processpool/pool.h"
#include "../src/actor.h"
#include "../src/message.h"
#include "squirrel.h"
#include "landcell.h"
#include "simclock.h"
#include "message_commands.h"
#include "sim_params.h"
#include "../src/log.h"

void simMasterCode();
void simWorkerCode();
void spawnProcess(int*, int, enum message_commands, int);
void send_clock_id(int*, int, int );

/* Called by the master process when spawned by processpool */
void simMasterCode()
{
	//Everything is an Actor. Will also help leverage the communication functions in this case
	int myRank;
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm_rank(comm, &myRank);
	initActor(myRank, comm);
	int id_landcell[LANDCELLS], id_clock[1];
	int id_healthy_squirrel[TOT_SQUIRRELS-INF_SQUIRRELS], id_inf_squirrel[INF_SQUIRRELS];

	/* 
	Initialising the Landcells first to make the grid cell ranks deterministic
	After master rank (0), the first 16 ranks will be Landcells. 
	When communicating with the Landcells, we will assume ranks 1-17 correspond to Landcells
	This easy assumption will ease some burdon of communication
	*/

	log_it("-------- Simulation Parameters -------- \n");
	log_it("Number of Grid Cells 			|\t%d\n", LANDCELLS);
	log_it("Number of Squirrels 			|\t%d\n", TOT_SQUIRRELS);
	log_it("Number of Inf. Squirrels 		|\t%d\n", INF_SQUIRRELS);
	log_it("Number of Months 				|\t%d\n", NUM_MONTHS);
	log_it("Length of a Month (seconds) 	|\t%f\n", MON_LENGTH);
	log_it("----------------------------------------\n");

	/* Init land cells */
	spawnProcess(id_landcell, LANDCELLS, INITIALISE_LAND, 1);

	/* Clock agent which will monitor the entire simulation and print out the output 
	 */
	spawnProcess(id_clock, 1, INITIALISE_CLOCK, 1);

	/* Spawn the initial number of squirrels with some of them infected */
	spawnProcess(id_healthy_squirrel, TOT_SQUIRRELS-INF_SQUIRRELS, INITIALISE_SQUIRREL, 0);
	spawnProcess(id_inf_squirrel, INF_SQUIRRELS, INITIALISE_SQUIRREL, 1); //infected

	/* Once initialized, send the squirrels the id of the clock actor*/
	send_clock_id(id_healthy_squirrel, TOT_SQUIRRELS-INF_SQUIRRELS, id_clock[0]);
	send_clock_id(id_inf_squirrel,  INF_SQUIRRELS, id_clock[0]);

	return;
}

/* Spawns the process and sends the initialization message to them */
void spawnProcess(int *id_arr, int numProcs, enum message_commands command, int data)
{
	int i, workerPid;
	for (i=0;i<numProcs;i++) {
		workerPid = startWorkerProcess();
		id_arr[i] = workerPid;
		msg_send(workerPid, command, data);
	}
}

/*Send clock id information to each process in the list */
void send_clock_id(int *actor_id, int size, int clock_id)
{
	//int size = sizeof(actor_id)/sizeof(int);
	int i;
	for (i=0;i<size;i++) {
		debug("Sending clock id to worker PID=%d\n", actor_id[i]);
		msg_send(actor_id[i], CLOCK_INFO, clock_id);
	}
}

void simWorkerCode()
{
	int workerStatus = 1;
	int myRank, parentId;
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm_rank(comm, &myRank);
	while(workerStatus) {
	//Everything is an Actor. Will also help leverage the communication functions in this case	
		initActor(myRank, comm);

		parentId = getCommandData();
		message_package in_msg;

		//Check for the message recd from the parent process
		msg_recv(&in_msg, parentId);
		switch(getMsgType(in_msg)) {
			case INITIALISE_LAND: {	land_main(); break; }
			case INITIALISE_CLOCK: { clock_main(); break; }
			case INITIALISE_SQUIRREL: {
				int infection_status = getMsgData(in_msg);
				squirrel_main(infection_status);
				break;
			}
		}//end switch
		workerStatus = workerSleep();
	}
	log_info("Worker Code ending for rank %s\n", myRank );
	return;
}
