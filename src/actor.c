/* Base class for Actors in the Actor pattern for parallelism
 * Actor attributes: id (rank), MPI Communicator, status (live or dead)
 * Primary functions: 
 	-	actor_birth : Give birth to new actor by starting a process on pool
 	-	actor_die: Kill existing actor by setting status = 0
 	-	msg_send : Use MPI_Bsend to fire-and-forget data to dest
 	-	msg_recv : Use MPI_Recv to issue a blocking receive for data
 	-	pending_recv: Use MPI_Iprobe to check if there are pending msgs on the queue
 	-	get methods for actor attributes : id, status, comm
 * Actor initialisation also registers the MPI data type to be used for communication
 */ 

#include <stdio.h>
#include "mpi.h"
#include "actor.h"
#include "message.h"
#include "log.h"
#include "../processpool/pool.h"

#define FALSE 0
#define TRUE  1
#define ACTOR_TAG 123

//The MPI type to be used for send and recv of the messages
static MPI_Datatype MSG_TYPE;

static actor thisActor;

//Internal functions
static void initialiseType();
int get_actor_id();

/* Initialize the status of this Actor 
   Also initializes the Message communication type to be used
*/
void initActor(int id, MPI_Comm comm) 
{
	thisActor.id = id;
	thisActor.comm = comm;
	thisActor.status = 1; //alive
	initialiseType();
	print_actor_state();
}

/* Give birth to a new actor on a new process 
   This project will only use MPI_COMM_WORLD for the comm
*/
int actor_birth() 
{
	int workerPid = startWorkerProcess();
	return workerPid;
}

/* Kill the worker by changing the flag to zero.
   Putting the worker to sleep doesn't work with this processpool in place.
 */
int actor_die()
{
	thisActor.status = 0;
	//int workerStatus = workerSleep();
	//return workerStatus;
	return thisActor.status;
}

/* Returns whether the actor is alive =1, or dead=0 */
int get_actor_status()
{
	return thisActor.status;
}

int get_actor_id()
{
	return thisActor.id;
}

MPI_Comm get_actor_comm()
{
	return thisActor.comm;
}

/* Initialises the MPI type for the message object to be used 
   by this actor. Re-using the function used in processpool (slightly modified)
*/
static void initialiseType() {
	message_package package;
	int blocklengths[4] = {1,1,1,1}, nitems=4;
	MPI_Datatype types[4] = {MPI_CHAR, MPI_INT, MPI_INT, MPI_INT};
	MPI_Aint offsets[4] = {0,4,8,12};
	MPI_Type_create_struct(nitems, blocklengths, offsets, types, &MSG_TYPE);
	MPI_Type_commit(&MSG_TYPE);
}

/* Sends the message_package to the destination use MPI_BSend
   The buffer required for BSend will be allocated at the simulation level
*/
void msg_send(int destination, char command, int data)
{
	message_package newMessage;
	newMessage.command_type = command;
	newMessage.data = data;
	newMessage.source = get_actor_id();
	newMessage.destination = destination;
	MPI_Bsend(&newMessage, 1, MSG_TYPE, destination, ACTOR_TAG, thisActor.comm);
}

/* Probes the source to check if there are any pending messages to be 
   received. If yes, we will issue the MPI_receive
*/
int pending_recv(int source) 
{
	int state;
	MPI_Status status;
	MPI_Iprobe(source, ACTOR_TAG, thisActor.comm, &state, &status);
	return state;
}

/* Blocking receive call. the msg will be read into the message_package 
*/
int msg_recv(message_package *newMessage, int source)
{
	message_package tempMessage;
	MPI_Recv(&tempMessage, 1, MSG_TYPE, source, ACTOR_TAG, thisActor.comm, MPI_STATUS_IGNORE);
	*newMessage = tempMessage;
}

/* Prints out the state of the actor to std err when compiled with DEBUG flag switch ON
*/
void print_actor_state()
{
	debug ("ActorID: %d, status=%d **\n", thisActor.id, thisActor.status);
}

/* Returns 1 if worker should stop, else 0. Calls the function inside the process pool */
int should_actor_stop()
{
	return shouldWorkerStop();
}
