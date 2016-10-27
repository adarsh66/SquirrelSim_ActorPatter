/*
	This is the implementation of the Simulation Clock Actor
	in the Squirrel Disease simulation
	There will only be one of these and it will control the passing of time
	Inform the others on the passing of months and print out the required output info
	to std out.
*/


#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../processpool/pool.h"
#include "../src/actor.h"
#include "../src/message.h"
#include "message_commands.h"
#include "sim_params.h"
#include "../src/log.h"

void initClock();
void clock_main();
void print_sim_details(int);

//The state info of simulation clock 
typedef struct simclock {
	int month_id;
	int cellPopInflux[LANDCELLS][NUM_MONTHS];
	int cellInfLevel[LANDCELLS][NUM_MONTHS];
	
	int num_squirrels;
	int num_inf_squirrels;
	int numSquirrels[NUM_MONTHS];
	int numInfSquirrels[NUM_MONTHS];

} simclock;

static simclock thisClock;

void initClock()
{
	thisClock.month_id = 0;
	int i,j;
	for (i=0;i<LANDCELLS;i++) {
		for(j=0;j<NUM_MONTHS;j++) {
			thisClock.cellPopInflux[i][j] = 0;
			thisClock.cellInfLevel[i][j] = 0;
			thisClock.numSquirrels[j] = 0;
			thisClock.numInfSquirrels[j] = 0;
		}	
	}

	//Initialize the values of month zero starting values
	thisClock.num_squirrels   = TOT_SQUIRRELS;
	thisClock.numSquirrels[0] = TOT_SQUIRRELS;

	thisClock.num_inf_squirrels  = INF_SQUIRRELS;
	thisClock.numInfSquirrels[0] = INF_SQUIRRELS;
}

void clock_main()
{
	initClock();

	int i,j, source, data, size;
	double starttime, currtime;
	starttime = MPI_Wtime();
	
	message_package in_msg;
	
	do {
		currtime = MPI_Wtime();
		if ((currtime - starttime) > MON_LENGTH) {
			//change month
			starttime = MPI_Wtime(); //reset the clock
			thisClock.month_id++;
			thisClock.numSquirrels[thisClock.month_id] = thisClock.num_squirrels;
			thisClock.numInfSquirrels[thisClock.month_id] = thisClock.num_inf_squirrels;
			//Hard coded grid cell destinations to 1 to 17 for now
			//TODO: think about how to communicate this info
			for (i=1;i<LANDCELLS+1;i++) {
				msg_send(i, CLOCK_MONTH, thisClock.month_id);
			}
			/* After sending the messages, this will receive
			 * We will issue two receives from each source separately
			 * They will be blocking receives
			 */
			for (i=1;i<LANDCELLS+1;i++)  {
				for (int j = 0; j< 2; j++)
				{
					msg_recv(&in_msg, i);
					source = getMsgSource(in_msg) -1;//MPI rank to grid cell conversion
					data = getMsgData(in_msg);
					switch(getMsgType(in_msg)) {
					case POPULATION_INFLUX: { thisClock.cellPopInflux[source][thisClock.month_id] = data; break; }
					case INFECTION_LEVEL: { thisClock.cellInfLevel[source][thisClock.month_id] = data; break; }
					}//end switch
				}//end 2nd for
			}//end recv for
		}//end if
		/* When not waiting for land cells, process can look at squirrel msgs */
		while (pending_recv(MPI_ANY_SOURCE)) {
			msg_recv(&in_msg, MPI_ANY_SOURCE);
			if (getMsgType(in_msg) == SQUIRREL_STATUS) {
				data = getMsgData(in_msg);
				switch (data) {
				case 1 : { thisClock.num_squirrels +=1; break;}
				case -1: { thisClock.num_squirrels -=1; thisClock.num_inf_squirrels -=1; break;}
				case 0 : { thisClock.num_inf_squirrels +=1; break; }
				}//end switch
			}
		}//end while recv 
	} while(thisClock.month_id < NUM_MONTHS-1); 

	print_sim_details(thisClock.month_id+1);
	
	//Theclock will inform all the other actors it is time to die
	//first check the current size of the world
	MPI_Comm_size(get_actor_comm(), &size);
	debug("MPI Size when calling for shutdown= %d\n",size );
	for (i=1;i<size;i++) { 
	//starting with i=1, as i=0 is root master process, and skip the id for this clock process :)
		if (i != get_actor_id()) {	msg_send(i, SIMULATION_END, 1); }
	}
	
	shutdownPool(); // Calls for a shut down of the process pool
}

/* Print the output of the clock simulation 
 * Output includes population influx and infection level on each cell in every month 
 * And it includes the number of squirrels alive and number infected for every month
 */
void print_sim_details(int month_id)
{
	int i, j;

	log_it("Cell\tMonth\tPopInf\tInfLvl\n");

	for (i=0;i<LANDCELLS;i++) {
		for(j=0;j<month_id;j++) {
			log_it("%d\t%d\t%d\t%d\n", i, j, 
				thisClock.cellPopInflux[i][j],
				thisClock.cellInfLevel[i][j] );
		}
	}
	//print out the squirrel population info
	log_it("Month\tSqAlive\tSqInfected\n");
	for (i=0;i<month_id;i++) {
		log_it("%d\t%d\t%d\n", i, thisClock.numSquirrels[i], thisClock.numInfSquirrels[i]);
	}
}

