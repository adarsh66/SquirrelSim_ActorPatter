/*	This is the implementation of the Squirrel Actor in the 
	Squirrel Disease simulation. This will leverage functionalities
	from the actor.c which is part of the actor framework
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "../processpool/pool.h"
#include "../src/actor.h"
#include "../src/message.h"
#include "../src/squirrel-functions.h"
#include "../src/ran2.h"
#include "message_commands.h"
#include "sim_params.h"
#include "../src/log.h"

#define NEW_SQUIRREL_TAG 456

typedef struct squirrel {
	float x;
	float y;
	int step_count;
	int infected_step_count;
	int visited_pop[WINDOWSIZE];
	int visited_inf[WINDOWSIZE];
	int infection_status;
	int clock_id;
} squirrel;

static squirrel thisSquirrel;
static long state;

//Internal functions
void initSquirrel(int);
int get_infection_status();
float get_avg(int*, int);
int squirrel_death();
void squirrel_birth();
void squirrel_infection();
void init_location();
void print_squirrel_state();
void get_clock_info();

/* Initialise a squirrel actor by passing in the process id, MPI Comm 
 * and starting infection status. This can be toggled for different squirrels 
 */
void initSquirrel(int infection_status) 
{
	get_clock_info();
	init_location();

	thisSquirrel.step_count = 0;
	thisSquirrel.infected_step_count = 0;

	thisSquirrel.infection_status = infection_status;
	int i;
	//Initialize the visit arrays to 0. before you start moving
	for (i=0;i<WINDOWSIZE;i++) { 
		thisSquirrel.visited_pop[i]=0; 
		thisSquirrel.visited_inf[i]=0; 
	}
}
/* Initialize the Squirrel with starting positions
 * If we are getting a new born squirrel, then we will receive its coords from parent
 * If we receive an initialized squirrel from master, we will not receive those coords
 */
void init_location()
{
	state = -1-get_actor_id();
	initialiseRNG(&state);
	float x_loc, y_loc;
	float squirrel_location[2] = {0.0,0.0};
	int parentId = getCommandData();

	if (parentId != 0) {
		MPI_Recv(squirrel_location, 2, MPI_FLOAT, parentId, NEW_SQUIRREL_TAG, get_actor_comm(), MPI_STATUS_IGNORE);
		thisSquirrel.x = squirrel_location[0];
		thisSquirrel.y = squirrel_location[1];
	} 
	else {
		squirrelStep(squirrel_location[0],squirrel_location[1],&x_loc, &y_loc, &state); //initialise poisitions
		thisSquirrel.x = x_loc;
		thisSquirrel.y = y_loc;	
	}
}

/* Read the initialization msg containing the clock id 
   And respond to the clock with the msg to indicate this squirrel is alive
 */
void get_clock_info()
{
	int parentId = getCommandData();
	message_package in_msg;
	msg_recv(&in_msg, parentId);
	printMsg(in_msg);
	thisSquirrel.clock_id = getMsgData(in_msg);
}

/* Main function which will be called by the worker when starting squirrel actor
 */
void squirrel_main(int infection_status)
{
	//Initialize the squirrel
	initSquirrel(infection_status);
	message_package in_msg;
	print_squirrel_state();
	
	//While alive, loop over this phase
	while(get_actor_status()) {
		//Take a step
		int destination = squirrel_step();
		//ASSUMPTION: landcells will be rank 1-17. function above will return values 0-16. 
		//So we will add 1 to the destination to get the rank of the land cell
		msg_send(destination+1, SQUIRREL_VISIT, thisSquirrel.infection_status);
		
		//After taking a step, now we receive any outstanding message
		while (pending_recv(MPI_ANY_SOURCE) & get_actor_status()){
			msg_recv(&in_msg, MPI_ANY_SOURCE);
			printMsg(in_msg);
			//Based on the type of msg, next steps
			switch(getMsgType(in_msg)) {
				case POPULATION_INFLUX: {
					/* Since we have split the msg reciept in two msgs, we need to consider only one of them as a step
					 * We have considered population influx msg as a sign that a step has been completed,
					 * All the computations associated with a squirrel and its steps will be done after this msg is recd
					 */
					thisSquirrel.step_count += 1;
					thisSquirrel.visited_pop[(thisSquirrel.step_count % WINDOWSIZE)] = getMsgData(in_msg);
					//Check if we have reached the WINDOWSIZE step threshold
					if ((thisSquirrel.step_count % WINDOWSIZE) == 0) {	
						squirrel_birth(); 
					}
					break;
				}
				case INFECTION_LEVEL: {
					thisSquirrel.visited_inf[(thisSquirrel.step_count % WINDOWSIZE)] = getMsgData(in_msg); 
					//Every "step" has completed, that is the infection level recd, 
					//and with squirrel having taken atleast 50 steps, 
					//we check infection status update and 
					if (thisSquirrel.infection_status == 0) {	
						if (thisSquirrel.step_count > WINDOWSIZE) squirrel_infection();
					} else {
						thisSquirrel.infected_step_count +=1;
						if (thisSquirrel.infected_step_count > WINDOWSIZE) {
							//check if it should die, more than WINDOWSIZE steps as an infected squirrel
							int result = squirrel_death();
						}
					}//end infection status update
					break;
				}
				case SIMULATION_END: {
					int returnval = actor_die();
					return;
				}
			}//end swtich
		} //end receive while		
	}//end main while actor is alive
	print_squirrel_state();
	log_info("FINALISE - Squirrel actor %d completed work\n",get_actor_id());
	return;
}

/* Squirrel take a step and return the cell position */
int squirrel_step()
{
	float x_loc, y_loc;

	squirrelStep(thisSquirrel.x, thisSquirrel.y, &x_loc, &y_loc, &state);
	thisSquirrel.x = x_loc; thisSquirrel.y = y_loc;

	return getCellFromPosition(x_loc, y_loc);
}

/* Will give birth to a new squirrel actor based on the squirrel-functions
 * If we give birth, we send the following info
 	-	To child: initial infection status to 0 
 	-	To child: current location (x,y) using standard MPI Bsend/recv (Framework doesnt handle this now)
 	-	To clock: Inform the clock actor regarding this birth
 * If the process pool is full, then simulation will not abort, instead, will continue
 * without spawning a child actor.
*/
void squirrel_birth()
{
	float avg_squirrel_population = get_avg(thisSquirrel.visited_pop, WINDOWSIZE);
	if (willGiveBirth(avg_squirrel_population, &state)) {
		// give birth
		int workerPid = actor_birth();
		if (workerPid >0) {
			msg_send(workerPid, INITIALISE_SQUIRREL, 0);
			log_info("Squirrel_ID=%d gave birth to Squirrel_ID=%d\n", get_actor_id(), workerPid );
			msg_send(workerPid, CLOCK_INFO, thisSquirrel.clock_id);
			msg_send(thisSquirrel.clock_id, SQUIRREL_STATUS, 1);
			//Send location info to new squirrels
			//HACK: Since framework doesnt support sending arrays, for now, we will use MPI commands directly
			float current_loc[2] = {thisSquirrel.x, thisSquirrel.y};
			MPI_Bsend(current_loc, 2, MPI_FLOAT, workerPid, NEW_SQUIRREL_TAG, get_actor_comm());
		} else { log_info("Squirrel_ID:%d could not spawn process as the pool is full!\n", get_actor_id());}
	}
	return;
}

/* Actor to be marked as dead to break the loop.
   Before dying squirrel will inform the clock on this life event
*/
int squirrel_death()
{
	int result = 1;
	if (willDie(&state)) {
		//data value of -1 indicates that squirrel is dead
		msg_send(thisSquirrel.clock_id, SQUIRREL_STATUS, -1);
		log_info("Squirrel_ID:%d to die. Infection Status=%d\n", get_actor_id(), thisSquirrel.infection_status );
		result = actor_die();
	}
	return result;
}

/* Will pass the avg infection rate to function inside squirrel-functions.c
   If squirrel deemed to have been infected, we will set the status of the squirrel to infected
   If infected, it will inform the clock about its change in infection level by sending value=0
   Which indicaes that squirrel has been infected
*/
void squirrel_infection()
{ 
	float avg_infection_rate = get_avg(thisSquirrel.visited_inf, WINDOWSIZE);
	if (willCatchDisease(avg_infection_rate, &state)) {
		//change infection status
		thisSquirrel.infection_status = 1;
		log_info("Squirrel_ID:%d has become infected\n", get_actor_id());
		//data value of 0 indicates infected status
		msg_send(thisSquirrel.clock_id, SQUIRREL_STATUS, 0);
	}
	return;
}

/* Calcs the average of a integer array 
   returns a float result
*/
float get_avg(int *arr, int size) {
	float result = 0.0;
	int i;
	for (i=0;i<size;i++) {
		result += arr[i];
	}
	return (float)result/size;
}

/* Print the state info of this Squirrel to std err when DEBUG compile flag is ON */
void print_squirrel_state()
{
	print_actor_state();
	int non_zero_pop=0, non_zero_inf=0;
	int i;
	for (i=0;i<WINDOWSIZE;i++) { if(thisSquirrel.visited_pop[i] >0) non_zero_pop++; }
	for (i=0;i<WINDOWSIZE;i++) { if(thisSquirrel.visited_inf[i] >0) non_zero_inf++; }
	debug("Squirrel_ID=%d, ClockID=%d, StepCount=%d, InfStepCount=%d, AvgInfLevl=%f, NonZeroVisitedSteps=%d, NonZeroVisitedInfected=%d\n", 
			get_actor_id(), thisSquirrel.clock_id, thisSquirrel.step_count, 
			thisSquirrel.infected_step_count, get_avg(thisSquirrel.visited_inf, WINDOWSIZE), 
			non_zero_pop, non_zero_inf);
}
