/*
	This is the implementation of the Land cell Actor
	within the Squirrel disease simulation. Will leverage
	functionalities from Actor patter script, actor.c
*/


#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../src/actor.h"
#include "../src/message.h"
#include "message_commands.h"
#include "sim_params.h"
#include "../src/log.h"

void initLandCell();
int sum_of_months (int*, int , int);
void land_main();
void print_cell_state();

typedef struct landcell {
	int visitors[NUM_MONTHS]; //Num of visitors for each month
	int inf_visitors[NUM_MONTHS]; //Num of infected visitors for each month
	int month_id;
	int population_influx;
	int infection_level;
} landcell;

static landcell thisLandCell;

//Initialise starting values for the Landcells
void initLandCell() 
{
	int i;
	for (i=0;i<NUM_MONTHS;i++) { thisLandCell.visitors[i]=0; thisLandCell.inf_visitors[i]=0; }
	thisLandCell.month_id = 0;
	thisLandCell.infection_level = thisLandCell.population_influx = 0;
}

int sum_of_months (int *arr, int month, int period) {
	int i, result =0;
	for (i=0;i<period;i++) {
		if ((month-i) >= 0) {
			result += arr[month-i];
		}
	}
	return result;
}
/* Main function which will loop till death */
void land_main()
{
	message_package in_msg;
	int infection_status;
	
	initLandCell();

	while(get_actor_status()) {
		while (pending_recv(MPI_ANY_SOURCE) & get_actor_status()){
			msg_recv(&in_msg, MPI_ANY_SOURCE);
			switch (getMsgType(in_msg)) {
				case SQUIRREL_VISIT: {
					printMsg(in_msg);
					/* If the simulation continues to run after final month, 
					 * it will continue till receiving a the END message, 
					 * simulation will continue to run, but will not update its parameters
					*/
					if (thisLandCell.month_id <= NUM_MONTHS-1) {
						thisLandCell.visitors[thisLandCell.month_id]++;
						infection_status = getMsgData(in_msg);
						thisLandCell.inf_visitors[thisLandCell.month_id] += infection_status; //=1 for infected, 0 for non infected
					}
					//Respond to the Squirrel with information
					msg_send(getMsgSource(in_msg), POPULATION_INFLUX, thisLandCell.population_influx);				
					msg_send(getMsgSource(in_msg), INFECTION_LEVEL, thisLandCell.infection_level);
					break;
				} //end squirrel_visit
				case CLOCK_MONTH: {
					thisLandCell.month_id += 1;
					thisLandCell.population_influx = sum_of_months(thisLandCell.visitors, thisLandCell.month_id, 3);
					thisLandCell.infection_level = sum_of_months(thisLandCell.inf_visitors, thisLandCell.month_id, 2);
					//Respond to the Clock cell with information
					msg_send(getMsgSource(in_msg), POPULATION_INFLUX, thisLandCell.population_influx);				
					msg_send(getMsgSource(in_msg), INFECTION_LEVEL, thisLandCell.infection_level);
					break;
				} //end clock visit
				case SIMULATION_END: {
					int returnval = actor_die();
					break;
				}
			}//end switch
		}//end while recv
	}//end main while
	log_info("FINALISE - Land actor %d Completed work\n",get_actor_id());
	return;
}

/* Print the state info of this Land Cell to std err if compiled with DEBUG flag ON*/
void print_cell_state()
{
	print_actor_state();

	debug("LAND_ID=%d, Month=%d, PopulationInflux=%d, InfectionLevel=%d\n", 
		get_actor_id(), thisLandCell.month_id, thisLandCell.population_influx, thisLandCell.infection_level);
}
					

